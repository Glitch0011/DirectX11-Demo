#pragma once

#include <d3d11.h>
#include <vector>
#include <Buffer.h>
#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>

#include <functional>

namespace SmoothGraphics
{
	class BufferController
	{
		concurrency::concurrent_unordered_map<std::wstring, Buffer*> buffers;
		concurrency::concurrent_unordered_map<std::wstring, Resource*> resources;

		/*concurrency::concurrent_vector<Buffer*> buffers;
		concurrency::concurrent_vector<Resource*> resources;*/
		
		ID3D11Device* device;

	public:
		BufferController(ID3D11Device* device);

		~BufferController();
		
		Buffer* Get(const std::wstring& name);
		Resource* GetResource(const std::wstring& name);

		void AddBuffer(Buffer*);
		void AddResource(Resource*);

		HRESULT Init();

		template<class T> HRESULT CreateDynamicConstantBuffer(const std::wstring& name)
		{
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));

			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = sizeof(T);
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			return CreateDynamicConstantBuffer(name, bd);
		}

		template<class T> HRESULT CreateConstantBuffer(const std::wstring& name)
		{
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));

			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(T);
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = 0;

			return CreateConstantBuffer(name, bd);
		}

		HRESULT CreateConstantBuffer(const std::wstring& name, D3D11_BUFFER_DESC constantBufferDesc);
		HRESULT CreateDynamicConstantBuffer(const std::wstring& name, D3D11_BUFFER_DESC constantBufferDesc);

		template<class T> HRESULT CreateStructuredBuffer(const std::wstring& name, UINT numberOfElements, T* data)
		{
			D3D11_BUFFER_DESC bufferDesc; ZeroMemory(&bufferDesc, sizeof(bufferDesc));
			bufferDesc.ByteWidth = numberOfElements * sizeof(T);
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bufferDesc.StructureByteStride = sizeof(T);

			D3D11_SUBRESOURCE_DATA bufferInitData; ZeroMemory(&bufferInitData, sizeof(bufferInitData));
			bufferInitData.pSysMem = data ? data : nullptr;

			ID3D11Buffer* ppBuffer;

			this->device->CreateBuffer(&bufferDesc, data ? &bufferInitData : nullptr, &ppBuffer);

			StructuredBuffer* buffer = new StructuredBuffer(name, ppBuffer, numberOfElements, sizeof(T));

			buffer->Init(this->device);

			this->AddBuffer(buffer);

			return S_OK;
		}

		template<class T> HRESULT CreateStructuredBuffer(const std::wstring& name, UINT numberOfElements, std::function<HRESULT(UINT, T*)> getPoint)
		{
			std::vector<T> verticies;

			if (getPoint)
			{
				verticies = std::vector<T>(numberOfElements);

				T simpleVertex; ZeroMemory(&simpleVertex, sizeof(T));

				UINT index = 0;
				while (index < numberOfElements && getPoint(index, &simpleVertex) == S_OK)
				{
					verticies[index++] = simpleVertex;
				}
			}

			CreateStructuredBuffer(name, numberOfElements, verticies.data());

			return S_OK;
		}
	};
}