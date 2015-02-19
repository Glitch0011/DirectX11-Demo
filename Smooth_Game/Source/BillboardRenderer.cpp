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
#include <PngFile.h>
#include <Hue.h>

float lerp(float v0, float v1, float t)
{
	return (1 - t)*v0 + t*v1;
}

float Distance(XMFLOAT3 a, XMFLOAT3 b)
{
	return sqrt(pow(a.x - b.x, 2.0f) + pow(a.y - b.y, 2.0f) + pow(a.z - b.z, 2.0f));
}

BillboardRendererComponent::BillboardRendererComponent(std::wstring textureName, UINT32 size)
	: SpriteRenderer(textureName)
{
	this->size = size;

	this->billboardData = CachedVariable<StructuredBuffer>(TO_FUNCTION(this->graphics->Get<StructuredBuffer>(L"BasicBillboard")));
	this->vShader = CachedVariable<VertexShader>(TO_FUNCTION(this->graphics->GetShader<VertexShader>(L"Instanced_VS")));

	this->functions[L"Render"] = [=](Params param) mutable
	{
		auto context = this->graphics->immediateContext;

		if (this->vShader == nullptr)
			return S_OK;

		this->vShader->SetInputLayout(context);

		this->vShader->Set(context);
		this->pShader->Set(context);

		context->VSSetConstantBuffers(0, 1, &this->basicConstantBuffer->buffer);
		context->VSSetConstantBuffers(1, 1, &this->quickConstantBuffer->buffer);

		//Calculate world matrix
		{
			if (this->positionData != nullptr)
			{
				auto worldMatrix =
					XMMatrixScalingFromVector(XMLoadFloat3(positionData->Scale())) *
					XMMatrixTranslationFromVector(XMLoadFloat3(positionData->Position())) *
					XMMatrixRotationX(positionData->Rotation()->x) *
					XMMatrixRotationY(positionData->Rotation()->y) *
					XMMatrixRotationZ(positionData->Rotation()->z);

				QuickBasicConstantBuffer b;
				b.mWorld = worldMatrix;

				this->quickConstantBuffer->Update(context, &b);
			}
		}

		//Set the shader resource
		context->VSSetShaderResources(0, 1, &this->billboardData->srvResourceView);

		//Set Top
 		this->graphics->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		this->indexBuffer->Set(context);
		this->vertexBuffer->Set(context);

		IF_NOT_NULL(samplerState, context->PSSetSamplers(0, 1, &samplerState.value));
		IF_NOT_NULL(texture, context->PSSetShaderResources(0, 1, &texture->srvResourceView));

		//Render
		this->graphics->immediateContext->DrawIndexedInstanced(6, this->size, 0, 0, 0);

		//Deset resources
		ID3D11ShaderResourceView* const g_pNullSRV = nullptr;
		context->VSSetShaderResources(0, 1, &g_pNullSRV);
		
		return S_OK;
	};

	this->functions[L"makeImage"] = [=](Params params)
	{
		std::shared_future<void> asyncFunction = async(launch::async, [=]()
		{
			auto newData = new vector<MovingParticleData>(this->size);

			SmoothHardware::PngFile pngFile(SmoothHardware::Path(L".\\Assets\\hi.png"));

			float ratio = (int)sqrt(this->size);
			int ratiox = ratio;
			int ratioy = ratio;
			float bX = pngFile.width / ratio;
			float bY = pngFile.height / ratio;

			std::vector<MovingParticleData*> shuffledData;
			for (auto& d : *newData)
				shuffledData.push_back(&d);

			auto engine = mt19937_64((unsigned int)time(0));
			std::shuffle(std::begin(shuffledData), std::end(shuffledData), engine);

			MovingParticleData** head = shuffledData.data();

			float spacing = 5.0f;
			float offset = -(((ratiox * bX) / 2) * spacing);
			for (int x = 0; x < ratiox; x++)
			{
				for (int y = 0; y < ratioy; y++)
				{
					(*head)->Target = XMFLOAT3(offset + ((x * bX) * spacing), offset + ((y * bY) * spacing), 0);
					auto c = pngFile.Sample((1.0f / (float)ratiox)*x, (1.0f / (float)ratioy)*y);
					(*head)->TargetColour = XMFLOAT4(c.r, c.g, c.b, c.a);
					(*head)->State = 1;
					(*head)->Player = 0;
					head++;
				}
			}

			this->DelayedSend(L"setData", { newData });
		});
		
		return S_OK;
	};

	this->functions[L"fluxColours"] = [=](Params params)
	{
		std::shared_future<void> asyncFunction = async(launch::async, [=]()
		{
			auto newData = new vector<MovingParticleData>(this->size);

			std::vector<MovingParticleData*> shuffledData;
			for (auto& d : *newData)
				shuffledData.push_back(&d);

			auto engine = mt19937_64((unsigned int)time(0));
			std::shuffle(std::begin(shuffledData), std::end(shuffledData), engine);

			mt19937_64 randomEngine; randomEngine.seed(clock());
			uniform_real_distribution<float> distribution(-50, 50);
			uniform_real_distribution<float> colourHDist(208, 260);
			uniform_real_distribution<float> colourSDist(100, 100);
			uniform_real_distribution<float> colourLDist(48, 52);

			MovingParticleData** head = shuffledData.data();

			for (auto i = 0; i < newData->size(); ++i)
			{
				(*head)->Target = XMFLOAT3(distribution(randomEngine), distribution(randomEngine), distribution(randomEngine));

				auto col = hsv2rgb(HsvColor(colourHDist(randomEngine), colourSDist(randomEngine), colourLDist(randomEngine)));

				(*head)->TargetColour = XMFLOAT4(col.r, col.g, col.b, 0.75f);
				(*head)->State = 2;
				(*head)->Player = 1;
				head++;
			}

			this->DelayedSend(L"setData", { newData });
		});

		return S_OK;
	};

	this->functions[L"pulse"] = [=](Params params)
	{
		std::shared_future<void> asyncFunction = async(launch::async, [=]()
		{
			
		});

		return S_OK;
	};

	this->functions[L"setData"] = [=](Params params)
	{
		auto data = (vector<MovingParticleData>*)params[0];

		this->graphics->Get<StructuredBuffer>(L"MovingParticleData")->
			Update(this->graphics->immediateContext, data->data());

		delete data;

		return S_OK;
	};
}

HRESULT BillboardRendererComponent::Init()
{
	auto res = SpriteRenderer::Init();

	mt19937_64 randomEngine; randomEngine.seed(clock());
	uniform_real_distribution<float> distribution(-495, 495);
	uniform_real_distribution<float> smallDist(0.0f, 1.0f);

	//Create Particle Data buffers
	this->graphics->CreateStructuredBuffer<SimpleVertex>(L"BasicBillboard", this->size, [&](UINT index, SimpleVertex* vertex)
	{
		vertex->Pos = XMFLOAT4((distribution)(randomEngine), (distribution)(randomEngine), 0, 1.0);
		vertex->Colour = XMFLOAT4((smallDist)(randomEngine), (smallDist)(randomEngine), (smallDist)(randomEngine), 1.0f);
		vertex->Vel = XMFLOAT4(0, 0, 0, 0.0);

		return S_OK;
	});
	
	this->graphics->CreateStructuredBuffer<MovingParticleData>(L"MovingParticleData", this->size, [&](UINT index, void* _vertex)
	{
		MovingParticleData* vertex = (MovingParticleData*)_vertex;
		vertex->State = 0;
		vertex->Player = 0;
		vertex->Target = XMFLOAT3(0.0, 0.0, 0.0);
		vertex->TargetColour = XMFLOAT4(0.5, 0.5, 0.5, 1.0);
		return S_OK;
	});

	//this->DelayedSend(L"fluxColours");

	return res;
}