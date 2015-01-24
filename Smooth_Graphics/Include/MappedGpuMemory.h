#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

using namespace DirectX;

namespace SmoothGraphics
{
	class MappedGpuMemory
	{
		ID3D11DeviceContext* context = nullptr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ID3D11Resource* buffer = nullptr;
		HRESULT result;

	public:
		MappedGpuMemory(ID3D11DeviceContext* context, ID3D11Resource* buffer, D3D11_MAP map_type = D3D11_MAP_WRITE_DISCARD)
		{
			this->context = context;
			this->buffer = buffer;

			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

			result = this->context->Map(buffer, 0, map_type, 0, &mappedResource);
		}

		~MappedGpuMemory()
		{
			if (SUCCEEDED(this->result))
			{
				this->context->Unmap(this->buffer, 0);
			}
		}

		void* Data()
		{
			return mappedResource.pData;
		}

		template<class T> HRESULT Set(T* data, UINT elementCount = 1)
		{
			if (SUCCEEDED(this->result) && this->mappedResource.pData != (void*)0x00000001)
			{
				memcpy(mappedResource.pData, data, sizeof(T) * elementCount);
				return S_OK;
			}
			else
			{
				return this->result;
			}
		}
	};
}