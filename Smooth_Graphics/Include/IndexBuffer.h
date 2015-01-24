#pragma once

#include <StructuredBuffer.h>

namespace SmoothGraphics
{
	class IndexBuffer : public StructuredBuffer
	{
	public:
		IndexBuffer(std::wstring name, ID3D11Buffer* buffer, UINT elementCount, UINT stride) :
			StructuredBuffer(name, buffer, elementCount, stride)
		{

		}

		virtual HRESULT Init(ID3D11Device* device)
		{
			return S_OK;
		}

		virtual HRESULT Set(ID3D11DeviceContext* immediateContext)
		{
			immediateContext->IASetIndexBuffer(this->Get(), DXGI_FORMAT_R32_UINT, this->offset);

			return StructuredBuffer::Set(immediateContext);
		}

		virtual HRESULT Draw(ID3D11DeviceContext* immediateContext)
		{
			immediateContext->DrawIndexed(this->elementCount, this->offset, 0);

			return S_OK;
		}
	};
}