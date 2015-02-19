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

	objectBuffer = CachedVariable<StructuredBuffer>(TO_FUNCTION(this->graphics->Get<StructuredBuffer>(L"BasicBillboard")));
	movingBuffer = CachedVariable<StructuredBuffer>(TO_FUNCTION(this->graphics->Get<StructuredBuffer>(L"MovingParticleData")));
	playerBuffer = CachedVariable<StructuredBuffer>(TO_FUNCTION(this->graphics->Get<StructuredBuffer>(L"PlayerData")));
	playerData = CachedVariable<vector<PlayerData>>(TO_FUNCTION(this->GameObject->SendAndRecieve<vector<PlayerData>*>(L"getPlayerData")));

	timeConstBuffer = CachedVariable<ConstantBuffer>(TO_FUNCTION(this->graphics->Get<ConstantBuffer>(L"TimeConstantBuffer")));
	elementConstBuffer = CachedVariable<ConstantBuffer>(TO_FUNCTION(this->graphics->Get<DynamicConstantBuffer>(L"ElementConstantBuffer")));

	this->functions[L"Update"] = [&](Params param)
	{
		auto context = this->graphics->immediateContext;

		if (playerBuffer->elementCount != playerData->size())
			return S_OK;

		playerBuffer->Update(context, playerData->data());
		timeConstBuffer->Update(context, (vector<double> {*(double*)param[0], *(double*)param[1]}).data());

		{
			ElementDataBuffer buffer;
			buffer.batchSize = DirectX::XMUINT4(this->batchSize.x, this->batchSize.y, this->batchSize.z, 0);
			elementConstBuffer->Update(context, &buffer);
		}

		return S_OK;
	};

	this->functions[L"Compute"] = [&](Params param)
	{
		std::wstring* shaderName = static_cast<std::wstring*>(param[0]);

		std::vector<ConstantBuffer*> buffers{ timeConstBuffer, elementConstBuffer };
		std::vector<StructuredBuffer*> structuredBuffers{ objectBuffer, movingBuffer, playerBuffer };

		if (param.size() > 1)
		{
			auto extraBuffers = static_cast<std::vector<ConstantBuffer*>*>(param[1]);
			for (auto buffer : *extraBuffers)
				buffers.push_back(buffer);
		}
		if (param.size() > 2)
		{
			auto extraBuffers = static_cast<std::vector<StructuredBuffer*>*>(param[2]);
			for (auto buffer : *extraBuffers)
				structuredBuffers.push_back(buffer);
		}

		this->Call(
			*shaderName,
			buffers,
			structuredBuffers,
			this->batchSize);

		return S_OK;
	};

	this->functions[L"ComputeRaw"] = [&](Params param)
	{
		std::wstring* shaderName = static_cast<std::wstring*>(param[0]);

		std::vector<ConstantBuffer*> buffers;
		std::vector<StructuredBuffer*> structuredBuffers;

		if (param.size() > 1)
		{
			auto extraBuffers = static_cast<std::vector<ConstantBuffer*>*>(param[1]);
			for (auto buffer : *extraBuffers)
				buffers.push_back(buffer);
		}
		if (param.size() > 2)
		{
			auto extraBuffers = static_cast<std::vector<StructuredBuffer*>*>(param[2]);
			for (auto buffer : *extraBuffers)
				structuredBuffers.push_back(buffer);
		}

		this->Call(
			*shaderName,
			buffers,
			structuredBuffers,
			this->batchSize);

		return S_OK;
	};
}

HRESULT ComputeComponent::Call(
	std::wstring& shaderName,
	std::vector<SmoothGraphics::ConstantBuffer*>& constantBuffers,
	std::vector<SmoothGraphics::StructuredBuffer*>& uavBuffers,
	DirectX::XMINT3& batchSize
	)
{
	auto context = this->graphics->immediateContext;

	auto shader = graphics->GetShader(shaderName);
	void* null = nullptr;
	for (int i = 0; i < constantBuffers.size(); i++)
		context->CSSetConstantBuffers(i, 1, constantBuffers[i] != nullptr ? &constantBuffers[i]->buffer : (ID3D11Buffer**)&null);
	for (int i = 0; i < uavBuffers.size(); i++)
		context->CSSetUnorderedAccessViews(i, 1, uavBuffers[i] != nullptr ? &uavBuffers[i]->uavResourceView : (ID3D11UnorderedAccessView**)&null, nullptr);
	
	graphics->DispatchComputeShader(batchSize, shader);

	void* blank = nullptr;

	for (int i = 0; i < constantBuffers.size(); i++)
		context->CSSetShaderResources(i, 1, (ID3D11ShaderResourceView**)&blank);
	for (int i = 0; i < uavBuffers.size(); i++)
		context->CSSetUnorderedAccessViews(i, 1, (ID3D11UnorderedAccessView**)&blank, nullptr);

	return S_OK;
}

HRESULT ComputeComponent::Init()
{
	auto res = Renderer::Init();

	this->graphics->bufferController->CreateDynamicConstantBuffer<TimeConstantBuffer>(L"TimeConstantBuffer");
	this->graphics->bufferController->CreateDynamicConstantBuffer<ElementDataBuffer>(L"ElementConstantBuffer");

	return res;
}