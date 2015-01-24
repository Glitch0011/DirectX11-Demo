#include <ConstantBuffer.h>

using namespace SmoothGraphics;

	
ConstantBuffer::ConstantBuffer(std::wstring name, ID3D11Buffer* buffer) : Buffer(name, buffer)
{

}

HRESULT ConstantBuffer::Update(ID3D11DeviceContext* context, void* data)
{
	context->UpdateSubresource(this->buffer, NULL, nullptr, data, NULL, NULL);
	return S_OK;
}