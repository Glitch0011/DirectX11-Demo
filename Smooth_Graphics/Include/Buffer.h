#pragma once

#include <d3d11.h>
#include <NamedResource.h>
#include <string>
#include <DirectXHelper.h>

namespace SmoothGraphics
{
	class Buffer : public NamedResource
	{
	public:
		ID3D11Buffer* buffer;

		Buffer(std::wstring name, ID3D11Buffer* buffer) :
			NamedResource(name)
		{
			SetDebugObjectName(this->buffer, this->name);

			this->buffer = buffer;
		}

		ID3D11Buffer* Get()
		{
			if (this)
				return this->buffer;
			else
				return nullptr;
		}

		virtual HRESULT Release()
		{
			this->buffer->Release();
			return S_OK;
		}
	};

	class Resource : public NamedResource
	{
	public:
		ID3D11Resource* resource;

		Resource(std::wstring name, ID3D11Resource* resource)
			: NamedResource(name)
		{
			SetDebugObjectName(this->resource, this->name);

			this->resource = resource;
		}

		ID3D11Resource* Get()
		{
			return resource;
		}

		virtual HRESULT Release()
		{
			this->resource->Release();
			return S_OK;
		}
	};
}