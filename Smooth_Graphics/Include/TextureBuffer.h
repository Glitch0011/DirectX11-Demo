#pragma once

#include <string>
#include <StructuredBuffer.h>

namespace SmoothGraphics
{
	class TextureBuffer : public Resource
	{
	public:
		ID3D11ShaderResourceView* srvResourceView = nullptr;
		ID3D11RenderTargetView* renderTargetView = nullptr;

		TextureBuffer(std::wstring name, ID3D11Resource* buffer) :
			Resource(name, buffer)
		{

		}

		ID3D11Texture2D* Get()
		{
			return (ID3D11Texture2D*) this->resource;
		}

		virtual HRESULT Init(ID3D11Device* device)
		{
			//Setup Shader Resource View
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));

			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;

			device->CreateShaderResourceView(this->resource, &srvDesc, &srvResourceView);
			if (this->srvResourceView)
				SetDebugObjectName(this->srvResourceView, this->name + L"_SRV");

			//Setup Render Target
			device->CreateRenderTargetView(this->resource, nullptr, &this->renderTargetView);

			if (this->renderTargetView)
				SetDebugObjectName(this->renderTargetView, this->name + L"_RTV");

			return S_OK;
		}
	};
}