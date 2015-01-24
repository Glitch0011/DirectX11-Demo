#include <LightController.h>

#include <SmoothGraphicsTypeDefs.h>

using namespace SmoothGraphics;
using namespace std;

HRESULT LightController::CreateLight(LightData* data)
{
	this->lightData[this->lightIndex++] = data;

	this->dirty = true;

	return S_OK;
}

HRESULT LightController::UpdateLight()
{
	this->dirty = true;

	return S_OK;
}

HRESULT LightController::UpdateBuffers(ID3D11DeviceContext* immediate)
{
	if (this->dirty)
	{
		if (lightDataBuffer && lightDataBuffer->elementCount == this->lightData.size() && this->lightDataBuffer->buffer)
		{
			/*auto mappedData = MappedGpuMemory(immediate, this->lightDataBuffer->buffer);
			mappedData.Set<LightData>(v.data(), this->lightData.size());*/

			auto v = this->getContiguousData();
			this->lightDataBuffer->Update(immediate, v.data());

			this->dirty = false;
		}
		else
		{
			IF_NOT_NULL(this->lightDataBuffer, this->lightDataBuffer->Release());

			this->bufferController->CreateStructuredBuffer<LightData>(L"lightData", this->lightData.size(), this->getContiguousData().data());
			this->lightDataBuffer = (StructuredBuffer*)this->bufferController->Get(L"lightData");
			this->dirty = false;
		}
	}

	return S_OK;
}