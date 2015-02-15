#include <ComputeComponent.h>

#include <GameObject.h>
#include <StructuredBuffer.h>

#include <ConstantBuffer.h>
#include <BillboardRendererComponent.h>

using namespace std;
using namespace SmoothGame;

ComputeComponent::ComputeComponent(XMINT3 batchSize)
{
	this->batchSize = batchSize;

	this->functions[L"Update"] = [&](Params param)
	{
		auto context = this->graphics->immediateContext;

		//Particle data
		auto data = (StructuredBuffer*)this->graphics->Get<StructuredBuffer>(L"BasicBillboard");
		auto movingData = (StructuredBuffer*)this->graphics->Get<StructuredBuffer>(L"MovingParticleData");

		auto playerData = this->graphics->Get<StructuredBuffer>(L"PlayerData");
		{
			auto player = this->GameObject->SendAndRecieve<vector<PlayerData>*>(L"getPlayerData");
			
			if (playerData->elementCount != player->size())
				return S_OK;

			playerData->Update(context, player->data());
		}

		void* blank = nullptr;

		//Setup the time constant buffer
		SmoothGraphics::ConstantBuffer* timeConstBuffer = this->graphics->Get<ConstantBuffer>(L"TimeConstantBuffer");
		timeConstBuffer->Update(context, (vector<double> {*(double*)param[0], *(double*)param[1]}).data());

		auto elementConstBuffer = this->graphics->Get<DynamicConstantBuffer>(L"ElementConstantBuffer");
		{
			ElementDataBuffer buffer;
			buffer.batchSize = DirectX::XMUINT4(this->batchSize.x, this->batchSize.y, this->batchSize.z, 0);
			elementConstBuffer->Update(context, &buffer);
		}

		auto shader = graphics->shaderController->Get(L"Basic_CS");
		context->CSSetConstantBuffers(0, 1, &timeConstBuffer->buffer);
		context->CSSetConstantBuffers(1, 1, &elementConstBuffer->buffer);
		context->CSSetUnorderedAccessViews(0, 1, &data->uavResourceView, nullptr);
		context->CSSetUnorderedAccessViews(1, 1, &movingData->uavResourceView, nullptr);
		context->CSSetUnorderedAccessViews(2, 1, &playerData->uavResourceView, nullptr);
		graphics->DispatchComputeShader(this->batchSize, shader);

		//Unset resources
		context->CSSetUnorderedAccessViews(0, 1, (ID3D11UnorderedAccessView**)&blank, nullptr);
		context->CSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&blank);
		context->CSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)&blank);

		return S_OK;
	};
}

HRESULT ComputeComponent::Init()
{
	auto res = Renderer::Init();

	this->graphics->bufferController->CreateDynamicConstantBuffer<TimeConstantBuffer>(L"TimeConstantBuffer");
	this->graphics->bufferController->CreateDynamicConstantBuffer<ElementDataBuffer>(L"ElementConstantBuffer");

	return res;
}