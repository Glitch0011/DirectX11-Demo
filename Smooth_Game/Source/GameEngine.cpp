#include <GameEngine.h>

#include <time.h>
#include <GraphicsEngine.h>
#include <ConstantBuffer.h>
#include <BasicConstantBuffer.h>
#include <PositionData.h>
#include <PositionComponent.h>

using namespace SmoothGame;
using namespace SmoothGraphics;
using namespace std;

GameEngine::GameEngine()
{
	memset(&this->inputStorage, 0, sizeof(InputStorage));
	ZeroMemory(&this->projectionMatrix, sizeof(XMFLOAT4X4));
	ZeroMemory(&this->frustum, sizeof(Frustum));
}

GameEngine::~GameEngine()
{
	this->graphics->CleanupDevice();

	delete this->graphics;

	this->onExit();
}

HRESULT GameEngine::Init(HWND g_hWnd)
{
	this->hWnd = g_hWnd;

	this->last = chrono::system_clock::now();
	this->timeStart = chrono::system_clock::now();

	this->graphics = new GraphicsEngine();

	this->graphics->onCreateScene = [this]()
	{
		this->calculatePerspectiveMatrix();

		if (this->onCreateScene)
			this->onCreateScene();

		this->Send(L"Init");

		return S_OK;
	};

	this->graphics->onRender = [this]()
	{
		this->UpdateConstantBuffers();

		for (auto& obj : this->objects)
			obj->Send(L"Render");

		if (this->onRender)
			this->onRender();

		return S_OK;
	};

	this->graphics->onUpdate = [this](const double& timePassed)
	{
		for (auto& obj : this->objects)
			if (obj->toSendMessages.size() > 0)
			{
				for (auto& msg : obj->toSendMessages)
					obj->Send(msg.first, msg.second);
				obj->toSendMessages.clear();
			}

		auto timeTotal = chrono::duration_cast<chrono::nanoseconds>(chrono::system_clock::now() - this->timeStart).count() * 1.0e-9;
		last = chrono::system_clock::now();

		for (auto& obj : this->objects)
			obj->Send(L"Update", { &((double)timePassed), &timeTotal });
		
		if (this->onUpdate)
			this->onUpdate(timePassed);

		return S_OK;
	};

	if (FAILED(graphics->InitDevice(g_hWnd)))
	{
		graphics->CleanupDevice();
		return 0;
	}

	return S_OK;
}

void GameEngine::calculatePerspectiveMatrix()
{
	if (this->graphics)
	{
		this->graphics->CalculateWidthAndHeight();

		float fov = XM_PIDIV4;
		float aspect = (float)this->graphics->width / (float)this->graphics->height;
		float farZ = 5000.0f;
		float nearZ = 1.0f;

		XMStoreFloat4x4(&this->projectionMatrix, XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ));

		frustum.setCamInternals(fov, aspect, nearZ, farZ);

		//this->graphics->Resize();
	}
}

void GameEngine::UpdateConstantBuffers()
{
	auto viewMatrix = XMMatrixIdentity();

	//Zoom out matrix
	viewMatrix *= XMMatrixTranslation(0, 0, 0);

	//Rotate around zoom
	viewMatrix *= XMMatrixRotationX(0);
	viewMatrix *= XMMatrixRotationY(0);

	//Get the Position Data
	if (this->Camera)
	{
		PositionalData* objData = this->Camera->SendAndRecieve<PositionalData*>(L"getPositionData");

		if (objData != nullptr)
		{
			//Apply global translation
			viewMatrix *= XMMatrixTranslation(objData->Position()->x, objData->Position()->y, objData->Position()->z);

			//Rotation
			viewMatrix *= XMMatrixRotationY(objData->Rotation()->y);
			viewMatrix *= XMMatrixRotationZ(objData->Rotation()->z);
			viewMatrix *= XMMatrixRotationX(objData->Rotation()->x);
		}

		//Scale
		viewMatrix *= XMMatrixScaling(1, 1, 1);

		BasicConstantBuffer buffer;

		//Load View
		buffer.mView = XMMatrixTranspose(viewMatrix);

		//Load Projection
		XMMATRIX projection = XMLoadFloat4x4(&projectionMatrix);
		buffer.mProjection = XMMatrixTranspose(projection);

		//Load Camera position
		auto pos = *objData->Position();
		pos.x *= -1; pos.y *= -1; pos.z *= -1;
		buffer.mCameraPosition = XMLoadFloat3(&pos);

		//Update the constant buffer
		auto constantBuffer = this->graphics->Get<ConstantBuffer>(L"BasicConstantBuffer");
		IF_NOT_NULL(constantBuffer, constantBuffer->Update(this->graphics->immediateContext, &buffer));
	}
}

GameObject* GameEngine::AddObject(MessageName name)
{
	return this->AddObject(name, {});
}

GameObject* GameEngine::AddObject(MessageName name, std::vector<Component*> components)
{
	auto obj = new GameObject(name);

	for (auto component : components)
		obj->AddComponent(component);

	obj->deadEndCallback = [=](MessageName functionName, Params params) { return this->DeadendCallback(functionName, params); };

	obj->Init();

	objects.push_back(obj);

	return objects.back();
}

GameObject* GameEngine::SetCamera(GameObject* gameObject)
{
	this->Camera = gameObject;
	return this->Camera;
}

void GameEngine::Draw()
{
	this->graphics->Render();
}

void GameEngine::Update()
{
	auto timePassedInSeconds = chrono::duration_cast<chrono::nanoseconds>(
		chrono::system_clock::now() - last).count() * 1.0e-9;

	last = chrono::system_clock::now();

	this->graphics->Update(timePassedInSeconds);

	if (nextFrameRate <= 0)
	{
		nextFrameRate = 1.0;
		lastFrameRate = frameRate;
		frameRate = 0;
	}
	else
	{
		nextFrameRate -= timePassedInSeconds;
		frameRate++;
	}

	//Create copy of the keys
	memcpy(this->inputStorage.lastKeys, this->inputStorage.keys, 253 * sizeof(int));
	memcpy(this->inputStorage.lastMouse, this->inputStorage.mouse, 3 * sizeof(int));

	if (this->GetCamera())
	{
		auto p = this->GetCamera()->GetComponent<PositionComponent>()->positionalData;
		auto position = XMLoadFloat3(p.Position()) * -1;

		auto f = p.ForwardVector();
		auto looking = position + XMLoadFloat3(&f);

		auto up = XMVectorSet(0, 1, 0, 0);

		this->frustum.setCamDef(position, looking, up);
	}

	return;
}

bool GameEngine::MouseEvent(MSG* msg)
{
	return true;
}

bool GameEngine::onKeyDown(WPARAM msg)
{
	this->inputStorage.keys[Key(msg)] = true;

	return true;
}

bool GameEngine::onKeyUp(WPARAM msg)
{
	this->inputStorage.keys[Key(msg)] = false;

	return true;
}

bool GameEngine::onMouseDown(MouseButtons button, WPARAM msg)
{
	switch (button)
	{
	case MouseButtons::Lmb:
		this->inputStorage.mouse[0] = true;
		break;
	case MouseButtons::Rmb:
		this->inputStorage.mouse[1] = true;
		break;
	}
	return true;
}

bool GameEngine::onMouseUp(MouseButtons button, WPARAM msg)
{
	switch (button)
	{
	case MouseButtons::Lmb:
		this->inputStorage.mouse[0] = false;
		break;
	case MouseButtons::Rmb:
		this->inputStorage.mouse[1] = false;
		break;
	}
	return true;
}

GameObject* GameEngine::GetCamera()
{
	return this->Camera;
}