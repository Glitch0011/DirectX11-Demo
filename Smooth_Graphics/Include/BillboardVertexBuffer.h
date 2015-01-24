#pragma once

#include <d3d11.h>
#include <VertexBuffer.h>
#include <string>
#include <DirectXMath.h>
#include <functional>
#include <SimpleVertex.h>
#include <DirectXHelper.h>

namespace SmoothGraphics
{
	class BillboardVertexBuffer : public VertexBuffer
	{
		std::function<HRESULT(int, SimpleVertex*)> getPoint;

	public:
		BillboardVertexBuffer(std::wstring name, std::function<HRESULT(int, SimpleVertex*)> getPoint, int count);

		D3D11_BUFFER_DESC GetDescription();

		HRESULT CreateBufferResourceView(ID3D11Device* device);

		HRESULT Init(ID3D11Device* device);

		HRESULT CreateSRV(ID3D11Device* device)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.ElementWidth = this->elementCount;
			
			HRESULT hr = S_OK;
			if (FAILED(hr = device->CreateShaderResourceView(this->buffer, &srvDesc, &this->srvResourceView)))
				return hr;

			SetDebugObjectName(this->srvResourceView, this->name + L"_SRV");

			return hr;
		}

		HRESULT CreateUAV(ID3D11Device* device)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			ZeroMemory(&uavDesc, sizeof(uavDesc));
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.NumElements = this->elementCount;

			HRESULT hr = S_OK;
			if (FAILED(hr = device->CreateUnorderedAccessView(this->buffer, &uavDesc, &this->uavResourceView)))
				return hr;

			SetDebugObjectName(this->srvResourceView, this->name + L"_SRV");

			return hr;
		}
	};
}