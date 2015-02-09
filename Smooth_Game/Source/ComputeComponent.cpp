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
		//Particle data
		auto data = (StructuredBuffer*)this->graphics->bufferController->Get(L"BasicBillboard");
		auto copy = (StructuredBuffer*)this->graphics->bufferController->Get(L"BasicBillboardCopy");
		auto movingData = (StructuredBuffer*)this->graphics->bufferController->Get(L"MovingParticleData");

		void* blank = nullptr;

		//Setup the time constant buffer
		auto timeConstBuffer = dynamic_cast<ConstantBuffer*>(this->graphics->bufferController->Get(L"TimeConstantBuffer"));
		timeConstBuffer->Update(this->graphics->immediateContext, (vector<double> {*(double*)param[0], *(double*)param[1]}).data());

		auto shader = graphics->shaderController->Get(L"Basic_CS");
		graphics->immediateContext->CSSetConstantBuffers(0, 1, &timeConstBuffer->buffer);
		graphics->immediateContext->CSSetUnorderedAccessViews(0, 1, &data->uavResourceView, nullptr);
		graphics->immediateContext->CSSetUnorderedAccessViews(1, 1, &movingData->uavResourceView, nullptr);
		graphics->DispatchComputeShader(this->batchSize, shader);

		//Setup resources for acceleration pass
		/*graphics->immediateContext->CSSetConstantBuffers(0, 1, &timeConstBuffer->buffer);
		graphics->immediateContext->CSSetUnorderedAccessViews(0, 1, &movingData->uavResourceView, nullptr);
		graphics->immediateContext->CSSetShaderResources(0, 1, &data->srvResourceView);
		graphics->DispatchComputeShader(batchSize, graphics->shaderController->Get(L"CalcAccel"));

		graphics->immediateContext->CSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&blank);

		//Copy particle data into backup buffer
		//graphics->immediateContext->CopyResource(copy->Get(), data->Get());

		//Setup resources for integration pass
		graphics->immediateContext->CSSetConstantBuffers(0, 1, &timeConstBuffer->buffer);
		graphics->immediateContext->CSSetUnorderedAccessViews(0, 1, &data->uavResourceView, nullptr);
		graphics->immediateContext->CSSetShaderResources(0, 1, &copy->srvResourceView);
		graphics->immediateContext->CSSetShaderResources(1, 1, &movingData->srvResourceView);
		graphics->DispatchComputeShader(batchSize, graphics->shaderController->Get(shaderName));*/

		//Unset resources

		graphics->immediateContext->CSSetUnorderedAccessViews(0, 1, (ID3D11UnorderedAccessView**)&blank, nullptr);
		graphics->immediateContext->CSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&blank);
		graphics->immediateContext->CSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)&blank);

		return S_OK;
	};
}

HRESULT ComputeComponent::Init()
{
	auto res = Renderer::Init();

	this->graphics->bufferController->CreateConstantBuffer<TimeConstantBuffer>(L"TimeConstantBuffer");

	return res;
}