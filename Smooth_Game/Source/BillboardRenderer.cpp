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

	samplerState = CachedVariable<ID3D11SamplerState>(TO_FUNCTION(this->graphics->samplerStates[L"samplerState"]));
	texture = CachedVariable<TextureBuffer>(TO_FUNCTION(this->graphics->GetResource<TextureBuffer>(L"texture")));

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
			auto context = this->graphics->immediateContext;

			auto shader = dynamic_cast<VertexShader*>((*graphics->shaderController)[L"Basic_VS"]);
			if (shader != nullptr)
				context->IASetInputLayout(shader->GetInputLayout());
			else
				return S_OK;

			//Setup the shaders
			this->graphics->shaderController->Get(L"Basic_VS")->Set(context);
			this->graphics->shaderController->Get(L"Basic_PS")->Set(context);

			//Apply the constant buffers
			context->VSSetConstantBuffers(0, 1, &this->graphics->bufferController->Get(L"BasicConstantBuffer")->buffer);
			context->VSSetConstantBuffers(1, 1, &this->graphics->bufferController->Get(L"QuickBasicConstantBuffer")->buffer);

			//Calculate world matrix
			auto worldMatrix =
				XMMatrixTranslationFromVector(XMLoadFloat3(objData->Position())) *
				XMMatrixScalingFromVector(XMLoadFloat3(objData->Scale())) *
				XMMatrixRotationX(objData->Rotation()->x) *
				XMMatrixRotationY(objData->Rotation()->y) *
				XMMatrixRotationZ(objData->Rotation()->z);

			auto scale = 10;
			auto src = XMFLOAT3(scale, scale, scale);
			worldMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&src));

			//Upload the world matrix to GPU

			{
				MappedGpuMemory mappedMemory(context, this->graphics->bufferController->Get(L"QuickBasicConstantBuffer")->Get());

				QuickBasicConstantBuffer b;
				b.mWorld = worldMatrix;

				mappedMemory.Set<QuickBasicConstantBuffer>(&b);
			}

			//Set the shader resource
			auto i = ((StructuredBuffer*)this->graphics->bufferController->Get(L"BasicBillboard"))->SRV();
			context->VSSetShaderResources(0, 1, &i);

			//Set Top
			this->graphics->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			//Render
			((IndexBuffer*)this->graphics->bufferController->Get(L"BasicIndexBuffer"))->Set(this->graphics->immediateContext);
			((VertexBuffer*)this->graphics->bufferController->Get(L"BasicVertexBuffer"))->Set(this->graphics->immediateContext);

			IF_NOT_NULL(samplerState, context->PSSetSamplers(0, 1, &samplerState.value));
			IF_NOT_NULL(texture, context->PSSetShaderResources(0, 1, &texture->srvResourceView));

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

			float spacing = 10.0f;
			float offset = -(((ratiox * bX) / 2) * spacing);
			for (int x = 0; x < ratiox; x++)
			{
				for (int y = 0; y < ratioy; y++)
				{
					(*head)->Accel = XMFLOAT3(0, 0, 0);
					(*head)->Target = XMFLOAT3(offset + ((x * bX) * spacing), offset + ((y * bY) * spacing), 0);
					auto c = pngFile.Sample((1.0f / (float)ratiox)*x, (1.0f / (float)ratioy)*y);
					(*head)->TargetColour = XMFLOAT4(c.r, c.g, c.b, c.a);
					head++;
				}
			}

			this->DelayedSend(L"setData", { newData });
		});
		
		return S_OK;
	};

	/*this->functions[L"makeLandspace"] = [=](Params params)
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
	};*/

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
	auto res = Renderer::Init();

	mt19937_64 randomEngine; randomEngine.seed(clock());
	uniform_real_distribution<float> distribution(-1000, 1000);
	uniform_real_distribution<float> smallDist(0.0f, 1.0f);

	//Create Particle Data buffers
	this->graphics->CreateStructuredBuffer<SimpleVertex>(L"BasicBillboard", this->size, [&](UINT index, SimpleVertex* vertex)
	{
		vertex->Pos = XMFLOAT4((distribution)(randomEngine), (distribution)(randomEngine), 0, 1.0);
		//vertex->Rotation = XMFLOAT3(0, 0, 0);
		//vertex->Scale = XMFLOAT3(1, 1, 1);

		vertex->Colour = XMFLOAT4((smallDist)(randomEngine), (smallDist)(randomEngine), (smallDist)(randomEngine), 1.0f);
		vertex->Vel = XMFLOAT4(0, 0, 0, 0.0);
		return S_OK;
	});
	this->graphics->CreateStructuredBuffer<SimpleVertex>(L"BasicBillboardCopy", this->size, nullptr);

	this->graphics->CreateStructuredBuffer<MovingParticleData>(L"MovingParticleData", this->size, [&](UINT index, void* _vertex)
	{
		MovingParticleData* vertex = (MovingParticleData*)_vertex;
		vertex->Accel = XMFLOAT3((smallDist)(randomEngine), (smallDist)(randomEngine), (smallDist)(randomEngine));
		vertex->Target = XMFLOAT3(0.0, 0.0, 0.0);
		return S_OK;
	});

	this->graphics->CreateVertexBuffer<SimpleRenderVertex>(L"BasicVertexBuffer", 4, [=](UINT index, SimpleRenderVertex* vertex)
	{
		SimpleRenderVertex v[]
		{
			SimpleRenderVertex(-0.5, -0.5, 0, 0),
			SimpleRenderVertex(-0.5, 0.5, 0, 1),
			SimpleRenderVertex(0.5, 0.5, 1, 1),
			SimpleRenderVertex(0.5, -0.5, 1, 0),
		};
		*vertex = v[index];
		return S_OK;
	});

	this->graphics->CreateIndexBuffer<int>(L"BasicIndexBuffer", 6, [=](UINT index, int* vertex)
	{
		int v[]
		{
			0, 1, 2,
			0, 3, 2,
		};
		*vertex = v[index];
		return S_OK;
	});

	return res;
}