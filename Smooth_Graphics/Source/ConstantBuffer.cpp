#include <ConstantBuffer.h>

using namespace SmoothGraphics;

#include <MappedGpuMemory.h>

ConstantBuffer::ConstantBuffer(std::wstring name, ID3D11Buffer* buffer) : Buffer(name, buffer)
{

}

HRESULT ConstantBuffer::Update(ID3D11DeviceContext* context, void* data)
{
	context->UpdateSubresource(this->buffer, NULL, nullptr, data, NULL, NULL);
	return S_OK;
}

DynamicConstantBuffer::DynamicConstantBuffer(std::wstring name, ID3D11Buffer* buffer) : ConstantBuffer(name, buffer)
{

}

HRESULT DynamicConstantBuffer::Update(ID3D11DeviceContext* context, void* data)
{
	auto mappedData = MappedGpuMemory(context, this->buffer);

	D3D11_BUFFER_DESC desc;
	this->buffer->GetDesc(&desc);

	memcpy(mappedData.Data(), data, desc.ByteWidth);

	return S_OK;
}