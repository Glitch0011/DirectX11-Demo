#pragma once

#include <d3d11.h>
#include <string>
#include <Buffer.h>

namespace SmoothGraphics
{
	class StructuredBuffer : public Buffer
	{
	public:
		ID3D11ShaderResourceView* srvResourceView = nullptr;
		ID3D11UnorderedAccessView* uavResourceView = nullptr;

		UINT elementCount;
		UINT stride;
		UINT offset;

		StructuredBuffer(std::wstring name, ID3D11Buffer* buffer, UINT elementCount, UINT stride) :
			Buffer(name, buffer)
		{
			this->elementCount = elementCount;
			this->stride = stride;
			this->offset = 0;
		}

		virtual HRESULT Init(ID3D11Device* device)
		{
			//Create Shader View
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.ElementWidth = elementCount;

			device->CreateShaderResourceView(this->buffer, &srvDesc, &srvResourceView);

			if (this->srvResourceView)
				SetDebugObjectName(this->srvResourceView, this->name + L"_SRV");

			// Create UAV
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			ZeroMemory(&uavDesc, sizeof(uavDesc));
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.NumElements = elementCount;

			device->CreateUnorderedAccessView(this->buffer, &uavDesc, &uavResourceView);

			if (this->uavResourceView)
				SetDebugObjectName(this->srvResourceView, this->name + L"_UAV");

			return S_OK;
		}

		void Update(ID3D11DeviceContext* context, void* data)
		{
			context->UpdateSubresource(this->buffer, NULL, nullptr, data, NULL, NULL);
		}

		virtual HRESULT Set(ID3D11DeviceContext* immediateContext)
		{
			return S_OK;
		}

		virtual HRESULT Release()
		{
			this->srvResourceView->Release();
			this->uavResourceView->Release();
			this->buffer->Release();

			return S_OK;
		}

		ID3D11ShaderResourceView* SRV()
		{
			return this->srvResourceView;
		}
		ID3D11UnorderedAccessView* UAV()
		{
			return this->uavResourceView;
		}
	};
}