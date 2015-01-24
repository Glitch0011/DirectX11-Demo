#pragma once

#include <string>
#include <StructuredBuffer.h>

namespace SmoothGraphics
{
	class VertexBuffer : public StructuredBuffer
	{
	public:
		VertexBuffer(std::wstring name, ID3D11Buffer* buffer, UINT elementCount, UINT stride) :
			StructuredBuffer(name, buffer, elementCount, stride)
		{

		}

		virtual HRESULT Init(ID3D11Device* device)
		{
			return S_OK;
		}

		virtual HRESULT Set(ID3D11DeviceContext* immediateContext)
		{
			immediateContext->IASetVertexBuffers(0, 1, &this->buffer, &this->stride, &this->offset);

			return StructuredBuffer::Set(immediateContext);
		}
	};
}