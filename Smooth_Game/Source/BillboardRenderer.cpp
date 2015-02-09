#include <BillboardRendererComponent.h>

#include <BasicConstantBuffer.h>
#include <random>
#include <time.h>
#include <PositionData.h>

#include <future>

using namespace SmoothGame;
using namespace SmoothGraphics;
using namespace std;

#include <GameObject.h>

float lerp(float v0, float v1, float t) 
{
	return (1 - t)*v0 + t*v1;
}

float Distance(XMFLOAT3 a, XMFLOAT3 b)
{
	return sqrt(pow(a.x - b.x, 2.0f) + pow(a.y - b.y, 2.0f) + pow(a.z - b.z, 2.0f));
}

BillboardRendererComponent::BillboardRendererComponent(int size) 
	: Renderer()
{
	XMFLOAT2 square_size = XMFLOAT2(500, 500);

	this->size = size;
		
	this->functions[L"Render"] = [=](Params param) mutable
	{
		if (this->objData == nullptr)
		{
			this->objData = (PositionalData*)0xFFFFFFFF;
			this->GameObject->SendAndRecieveAsync<PositionalData*>(L"getPositionData", [=](PositionalData* pos)
			{
				this->objData = pos;
			});
		}
		else if (this->objData != (PositionalData*)0xFFFFFFFF)
		{
			auto shader = dynamic_cast<VertexShader*>((*graphics->shaderController)[L"Basic_VS"]);
			if (shader != nullptr)
				this->graphics->immediateContext->IASetInputLayout(shader->GetInputLayout());
			else
				return S_OK;

			//Setup the shaders
			this->graphics->shaderController->Get(L"Basic_VS")->Set(this->graphics->immediateContext);
			this->graphics->shaderController->Get(L"Basic_PS")->Set(this->graphics->immediateContext);

			//Apply the constant buffers
			this->graphics->immediateContext->VSSetConstantBuffers(0, 1, &this->graphics->bufferController->Get(L"BasicConstantBuffer")->buffer);
			this->graphics->immediateContext->VSSetConstantBuffers(1, 1, &this->graphics->bufferController->Get(L"QuickBasicConstantBuffer")->buffer);

			//Calculate world matrix
			auto worldMatrix =
				XMMatrixTranslationFromVector(XMLoadFloat3(objData->Position())) *
				XMMatrixScalingFromVector(XMLoadFloat3(objData->Scale())) *
				XMMatrixRotationX(objData->Rotation()->x) *
				XMMatrixRotationY(objData->Rotation()->y) *
				XMMatrixRotationZ(objData->Rotation()->z);

			worldMatrix = XMMatrixIdentity();

			//Upload the world matrix to GPU

			{
				MappedGpuMemory mappedMemory(this->graphics->immediateContext, this->graphics->bufferController->Get(L"QuickBasicConstantBuffer")->Get());

				QuickBasicConstantBuffer b;
				b.mWorld = worldMatrix;

				mappedMemory.Set<QuickBasicConstantBuffer>(&b);
			}

			//Set the shader resource
			auto i = ((StructuredBuffer*)this->graphics->bufferController->Get(L"BasicBillboard"))->SRV();
			this->graphics->immediateContext->VSSetShaderResources(0, 1, &i);

			//Set Top
			this->graphics->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			//Render
			((IndexBuffer*)this->graphics->bufferController->Get(L"BasicIndexBuffer"))->Set(this->graphics->immediateContext);
			((VertexBuffer*)this->graphics->bufferController->Get(L"BasicVertexBuffer"))->Set(this->graphics->immediateContext);

			this->graphics->immediateContext->DrawIndexedInstanced(6, this->size, 0, 0, 0);

			ID3D11ShaderResourceView* const g_pNullSRV = nullptr;
			ID3D11Buffer* const g_pNullBuffer = nullptr;
			UINT g_iNullUINT = 0;

			//Deset resources
			this->graphics->immediateContext->VSSetShaderResources(0, 1, &g_pNullSRV);
			this->graphics->immediateContext->IASetVertexBuffers(0, 1, &g_pNullBuffer, &g_iNullUINT, &g_iNullUINT);
		}
		return S_OK;
	};

	/*this->functions[L"makeModel"] = [=](Params params)
	{
		std::shared_future<void> asyncFunction = async(launch::async, [=]()
		{
			auto model = this->graphics->modelController->Get(L"su-122-54");

			if (model)
			{
				auto data = new vector<MovingParticleData>(this->size);

				for (UINT i = 0; i < data->size(); i++)
				{
					(*data)[i].Target = XMFLOAT3(0, 0, 0);
					(*data)[i].Accel = XMFLOAT3(0, 0, 0);
				}

				Shape* shape = model->shapes[0];

				//Set to the mandatory points
				for (UINT i = 0; i < shape->positions.size(); i++)
				{
					(*data)[i].Target = shape->positions[i];
					(*data)[i].Target.x *= 50;
					(*data)[i].Target.y *= 50;
					(*data)[i].Target.z *= 50;
				}

				auto randomEngine = new mt19937_64(); randomEngine->seed(clock());
				auto distribution = new uniform_real_distribution<float>(0.0f, data->size());
				auto triDistribution = new uniform_real_distribution<float>(0.0f, shape->positions.size() - 3);
				auto floatDistribution = new uniform_real_distribution<float>(0.0f, 1.0f);

				for (UINT i = shape->positions.size(); i < data->size(); i++)
				{
					auto tri = (*triDistribution)(*randomEngine);

					auto pos = XMFLOAT3(0, 0, 0);

					pos.x = lerp(shape->positions[tri].x, shape->positions[tri + 1].x, (*floatDistribution)(*randomEngine));
					pos.y = lerp(shape->positions[tri].y, shape->positions[tri + 1].y, (*floatDistribution)(*randomEngine));
					pos.z = lerp(shape->positions[tri].z, shape->positions[tri + 1].z, (*floatDistribution)(*randomEngine));

					pos.x = lerp(pos.x, shape->positions[tri + 1].x, (*floatDistribution)(*randomEngine));
					pos.y = lerp(pos.y, shape->positions[tri + 2].y, (*floatDistribution)(*randomEngine));
					pos.z = lerp(pos.z, shape->positions[tri + 2].z, (*floatDistribution)(*randomEngine));

					(*data)[i].Target = pos;
					(*data)[i].Target.x *= 50;
					(*data)[i].Target.y *= 50;
					(*data)[i].Target.z *= 50;

					auto colour = 1.0 * max(Distance(pos, shape->positions[tri + 0]), max(Distance(pos, shape->positions[tri + 1]), Distance(pos, shape->positions[tri + 2])));
					(*data)[i].TargetColour = XMFLOAT4(colour, colour, colour, 1);
					(*data)[i].TargetRotation = XMFLOAT3((*distribution)(*randomEngine) * XM_2PI, (*distribution)(*randomEngine) * XM_2PI, (*distribution)(*randomEngine) * XM_2PI);
					(*data)[i].TargetScale = XMFLOAT3((*floatDistribution)(*randomEngine), (*floatDistribution)(*randomEngine), (*floatDistribution)(*randomEngine));
				}

				this->DelayedSend(L"setLandspace", { data }); 
			}
		});

		return S_OK;
	};

	this->functions[L"makeCenter"] = [=](Params params)
	{
		std::shared_future<void> asyncFunction = async(launch::async, [=]()
		{
			XMFLOAT2 square_size = XMFLOAT2(10, 10);

			auto randomEngine = new mt19937_64(); randomEngine->seed(clock());
			auto distribution = new uniform_real_distribution<float>(0.0f, 1.0f);

			auto data = new vector<MovingParticleData>(this->size);
			for (UINT i = 0; i < data->size(); i++)
			{
				(*data)[i].Accel = XMFLOAT3(0, 0, 0);

				auto radius = 100;
				auto theta = 2 * XM_PI * (*distribution)(*randomEngine);
				auto phi = acos(2 * (*distribution)(*randomEngine) - 1);
				auto x = 0 + (radius * sin(phi) * cos(theta));
				auto y = 0 + (radius * sin(phi) * sin(theta));
				auto z = 0 + (radius * cos(phi));

				auto angle = (360.0 / (double)data->size()) * i;
				(*data)[i].Target = XMFLOAT3(x, y, z);

				(*data)[i].TargetColour = XMFLOAT4(sin((*data)[i].Target.y), 0.1, 0.1, 0);
				(*data)[i].TargetRotation = XMFLOAT3((*distribution)(*randomEngine) * XM_2PI, (*distribution)(*randomEngine) * XM_2PI, (*distribution)(*randomEngine) * XM_2PI);
				auto s = sin(x/5) * 5;
				if (s < 0.5)
					s = 0;
				(*data)[i].TargetScale = XMFLOAT3(s, s, s);
			}

			this->DelayedSend(L"setLandspace", { data });
		});

		return S_OK;
	};

	this->functions[L"makeFlat"] = [=](Params params)
	{
		std::shared_future<void> asyncFunction = async(launch::async, [=]()
		{
			auto newData = new vector<MovingParticleData>(this->size);

			auto randomEngine = new mt19937_64(); randomEngine->seed(clock());
			auto distribution = new uniform_real_distribution<float>(0.0f, 1.0f);

			for (UINT i = 0; i < newData->size(); i++)
			{
				(*newData)[i].Accel = XMFLOAT3(0, 0, 0);

				XMFLOAT2 spacing = XMFLOAT2((float)(square_size.x / sqrt(this->size)), (float)(square_size.y / sqrt(this->size)));
				auto perRow = sqrt(this->size);

				auto x = (float)fmod(i, perRow) * spacing.x;
				auto z = (float)floor(i / perRow) * spacing.y;

				auto pos = XMFLOAT3(
					-(((float)perRow * spacing.x) / 2.0f) + x,
					(sin(x / 50.0) + cos(z / 50.0)) * 25,
					-(((float)perRow * spacing.y) / 2.0f) + z);

				(*newData)[i].Target = pos;

				(*newData)[i].TargetColour = XMFLOAT4(sin(pos.x / 25.0f), cos(pos.y / 50.0f), cos(pos.z / 25.0f), 1);
				(*newData)[i].TargetRotation = XMFLOAT3((*distribution)(*randomEngine) * XM_2PI, (*distribution)(*randomEngine) * XM_2PI, (*distribution)(*randomEngine) * XM_2PI);
				(*newData)[i].TargetScale = XMFLOAT3(0.5f,0.5f,0.5f);
			}

			this->DelayedSend(L"setLandspace", { newData });
		});
		
		return S_OK;
	};

	this->functions[L"makeLandspace"] = [=](Params params)
	{
		std::shared_future<void> asyncFunction = async(launch::async, [=]()
		{
			module::Perlin myModule;
			myModule.SetSeed(clock());

			utils::NoiseMap heightMap;
			utils::NoiseMapBuilderPlane heightMapBuilder;
			heightMapBuilder.SetSourceModule(myModule);
			heightMapBuilder.SetDestNoiseMap(heightMap);
			heightMapBuilder.SetDestSize((int)square_size.x, (int)square_size.y);
			heightMapBuilder.SetBounds(2.0, 6.0, 1.0, 5.0);
			heightMapBuilder.Build();

			auto newData = new vector<MovingParticleData>(this->size);

			auto randomEngine = new mt19937_64(); randomEngine->seed(clock());
			auto distribution = new uniform_real_distribution<float>(0.0f, 1.0f);

			for (UINT i = 0; i < newData->size(); i++)
			{
				(*newData)[i].Accel = XMFLOAT3(0, 0, 0);

				XMFLOAT2 spacing = XMFLOAT2((float)(square_size.x / sqrt(this->size)), (float)(square_size.y / sqrt(this->size)));
				auto perRow = sqrt(this->size);

				auto x = (float)fmod(i, perRow) * spacing.x;
				auto z = (float)floor(i / perRow) * spacing.y;

				auto a = heightMap.GetValue((int)x, (int)z) * 50;

				(*newData)[i].Target = XMFLOAT3(
					-(((float)perRow * spacing.x) / 2.0f) + x,
					((1 + heightMap.GetValue((int)x, (int)z))/2) * 100,
					-(((float)perRow * spacing.y) / 2.0f) + z);

				(*newData)[i].TargetColour = XMFLOAT4(0.1, ((*newData)[i].Target.y - 10) / 50, 0.1, 1);
				(*newData)[i].TargetScale = XMFLOAT3(1,1,1);
				(*newData)[i].TargetRotation = XMFLOAT3((*distribution)(*randomEngine) * XM_2PI, (*distribution)(*randomEngine) * XM_2PI, (*distribution)(*randomEngine) * XM_2PI);
			}

			this->DelayedSend(L"setLandspace", { newData });
		});

		return S_OK;
	};

	this->functions[L"setLandspace"] = [=](Params params)
	{
		auto data = (vector<MovingParticleData>*)params[0];

		this->graphics->immediateContext->UpdateSubresource(
			this->graphics->bufferController->Get(L"MovingParticleData")->buffer,
			0,
			nullptr,
			data->data(),
			sizeof(MovingParticleData),
			0);

		delete data;

		return S_OK;
	};*/
}

HRESULT BillboardRendererComponent::Init()
{
	auto res = Renderer::Init();

	mt19937_64 randomEngine; randomEngine.seed(clock());
	uniform_real_distribution<float> distribution(-10, 10);
	uniform_real_distribution<float> smallDist(0.0f, 1.0f);

	//Create Particle Data buffers
	this->graphics->CreateStructuredBuffer<SimpleVertex>(L"BasicBillboard", this->size, [&](UINT index, SimpleVertex* vertex)
	{
		vertex->Pos = XMFLOAT4(0, 0, 0, 1.0);
		vertex->Rotation = XMFLOAT3(0, 0, 0);
		vertex->Scale = XMFLOAT3(1, 1, 1);

		vertex->Colour = XMFLOAT4((smallDist)(randomEngine), (smallDist)(randomEngine), (smallDist)(randomEngine), 1.0f);
		vertex->Vel = XMFLOAT4(0, 0, 0, 0.0);
		return S_OK;
	});
	this->graphics->CreateStructuredBuffer<SimpleVertex>(L"BasicBillboardCopy", this->size, nullptr);

	this->graphics->CreateStructuredBuffer<MovingParticleData>(L"MovingParticleData", this->size, [&](UINT index, void* _vertex)
	{
		MovingParticleData* vertex = (MovingParticleData*)_vertex;
		vertex->Accel = XMFLOAT3((smallDist)(randomEngine), (smallDist)(randomEngine), (smallDist)(randomEngine));
		vertex->Target = XMFLOAT3(0.0, 0, 0.0);
		return S_OK;
	});

	this->graphics->CreateVertexBuffer<XMFLOAT3>(L"BasicVertexBuffer", 4, [=](UINT index, XMFLOAT3* vertex)
	{
		XMFLOAT3 v[]
		{
			XMFLOAT3(-0.5f, -0.5f, 0.5f),
			XMFLOAT3(-0.5f, 0.5f, 0.5f),
			XMFLOAT3(0.5f, 0.5f, 0.5f),
			XMFLOAT3(0.5f, -0.5f, 0.5f),
		};
		*vertex = v[index];
		return S_OK;
	});

	this->graphics->CreateIndexBuffer<int>(L"BasicIndexBuffer", 6, [=](UINT index, int* vertex)
	{
		int v[]
		{
			0, 1, 2,
			0, 2, 3,
		};
		*vertex = v[index];
		return S_OK;
	});

	return res;
}