#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <SimpleVertex.h>
#include <string>
#include <array>
#include <vector>
#include <fstream>
#include <memory>

#include <ShaderController.h>
#include <VertexBufferController.h>

#include <functional>
#include <StructuredBuffer.h>
#include <ModelController.h>
#include <MessageController.h>
#include <LightController.h>

#include <VertexBuffer.h>
#include <IndexBuffer.h>
#include <TextureBuffer.h>
#include <DepthTextureBuffer.h>
#include <ConstantBuffer.h>

#include <Directory.h>

#include <BmpFile.h>

#include <SmoothGraphicsTypeDefs.h>
#include <CachedVariable.h>
#include <PickingData.h>

using namespace DirectX;

namespace SmoothGraphics
{
	class GraphicsEngine
	{
	public:
		D3D_DRIVER_TYPE            driverType = D3D_DRIVER_TYPE_NULL;
		D3D_FEATURE_LEVEL          featureLevel = D3D_FEATURE_LEVEL_11_0;

		ID3D11Device*              device = nullptr;
		ID3D11DeviceContext*       immediateContext = nullptr;
		IDXGISwapChain*            swapChain = nullptr;
		ID3D11RenderTargetView*    renderTargetView = nullptr;

		ID3D11Texture2D*           depthStencil = nullptr;
		ID3D11DepthStencilState*   depthStencilState = nullptr;

		ID3D11RasterizerState*     rasterState = nullptr;
		ID3D11SamplerState*        sampleState = nullptr;

		std::function<HRESULT()>   onCreateScene = nullptr;
		std::function<HRESULT()>   onRender = nullptr;
		std::function<HRESULT(const double&)>   onUpdate = nullptr;

		std::function<void(const std::wstring& msg, HRESULT res)> onError = nullptr;

		std::map<std::wstring, ID3D11SamplerState*> samplerStates;

		UINT width, height;

		HWND hWnd;

		ShaderController* shaderController;
		VertexBufferController* vertexBufferController;
		ModelController* modelController;
		BufferController* bufferController;
		LightController* lightController;

		CachedVariable<TextureBuffer> colourRenderTarget, normalRenderTarget, depthRenderTarget, pickingRenderTarget;
		CachedVariable<DepthTextureBuffer> stencilBuffer;
		CachedVariable<StructuredBuffer> lightData;
		CachedVariable<VertexBuffer> quadVertexBuffer;
		CachedVariable<IndexBuffer> quadIndexBuffer;
		CachedVariable<ConstantBuffer> quadBasicConstantBuffer;
		CachedVariable<ID3D11SamplerState> quadSamplerState;

		HRESULT InitDevice(HWND g_hWnd);

		HRESULT CleanupDevice();

		HRESULT Render();

		HRESULT CreateDeviceAndSwapChain();

		HRESULT CreateRenderTargetView();

		HRESULT CreateViewport();

		HRESULT CreateDepthStencilState();

		HRESULT CreateSamplerState();

		HRESULT UnsetVertexBuffer();

		HRESULT UnsetUnorderedAccessViews();

		HRESULT DispatchComputeShader(XMINT3 dimensions, Shader* shader);

		HRESULT Resize();

		HRESULT CalculateWidthAndHeight();

		HRESULT Update(const double&);

		HRESULT CreateRasterState();
		
		HRESULT CreateDepthStencilView();

		HRESULT CreateColourTexture();

		HRESULT CreateNormalTexture();

		HRESULT CreateDepthTexture();

		HRESULT CreatePickingTexture();

		HRESULT CreatePickingStagingTexture();

		HRESULT CreateRenderTargetView(ID3D11Texture2D* buffer, ID3D11RenderTargetView** view)
		{
			return device->CreateRenderTargetView(buffer, nullptr, view);
		}

		HRESULT DrawDeferredQuad();

		HRESULT SetRenderTargetToBackBuffer();
		HRESULT SetDeferredRenderTargets();

		XMINT2 GetSize()
		{
			ID3D11Texture2D* pBackBuffer = nullptr;

			HRESULT err;

			this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

			D3D11_TEXTURE2D_DESC backBufferSurfaceDesc;
			pBackBuffer->GetDesc(&backBufferSurfaceDesc);

			pBackBuffer->Release();

			return XMINT2(backBufferSurfaceDesc.Width, backBufferSurfaceDesc.Height);
		}

		ID3D11SamplerState* GetSamplerState(const std::wstring& name)
		{
			for (auto sampler : this->samplerStates)
				if (sampler.first == name)
					return sampler.second;
			
			return nullptr;
		}

		template<class T = Buffer>T* Get(const std::wstring& name)
		{
			return static_cast<T*>(this->bufferController->Get(name));
		}

		template<class T = Resource>T* GetResource(const std::wstring& name)
		{
			return static_cast<T*>(this->bufferController->GetResource(name));
		}

		template<class T = Shader>T* GetShader(const std::wstring&name)
		{
			return static_cast<T*>(this->shaderController->Get(name));
		}

		template<class T> HRESULT CreateStructuredBuffer(const std::wstring& name, UINT numberOfElements, std::function<HRESULT(UINT, T*)> getPoint)
		{
			return this->bufferController->CreateStructuredBuffer(name, numberOfElements, getPoint);
		}

		template<class T = TextureBuffer>HRESULT CreateTexture(const std::wstring& name, D3D11_TEXTURE2D_DESC& desc, void* data = nullptr)
		{
			ID3D11Texture2D* texture = nullptr;

			D3D11_SUBRESOURCE_DATA dataStruct; ZeroMemory(&dataStruct, sizeof(D3D11_SUBRESOURCE_DATA));
			if (data)
			{
				dataStruct.SysMemPitch = desc.Width * (sizeof(float) * 4);
				dataStruct.pSysMem = data;
			}

			HRESULT hr;
			if (FAILED(hr = this->device->CreateTexture2D(&desc, data ? &dataStruct : nullptr, &texture)))
				return hr;

			auto textureBuffer = new T(name, texture);

			this->bufferController->AddResource(textureBuffer);

			return textureBuffer->Init(this->device);
		}

		HRESULT CreateStencilTexture();

		template<class T> HRESULT SetLightData(UINT count, std::function<T(UINT index)> getData)
		{
			this->CreateStructuredBuffer<LightData>(L"lightData", 9, [=](UINT i, LightData* data)
			{
				*data = getData(i);
				return S_OK;
			});

			return S_OK;
		}

		template<class T> HRESULT CreateVertexBuffer(const std::wstring& name, UINT numberOfElements, std::function<HRESULT(UINT, T*)> getPoint)
		{
			vector<T> verticies;

			if (getPoint)
			{
				verticies = vector<T>(numberOfElements);

				T simpleVertex; ZeroMemory(&simpleVertex, sizeof(T));

				UINT index = 0;
				while (index < numberOfElements && getPoint(index, &simpleVertex) == S_OK)
				{
					verticies[index++] = simpleVertex;
				}
			}

			D3D11_BUFFER_DESC bufferDesc;
			ZeroMemory(&bufferDesc, sizeof(bufferDesc));
			bufferDesc.ByteWidth = numberOfElements * sizeof(T);
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = sizeof(T);

			D3D11_SUBRESOURCE_DATA bufferInitData;
			ZeroMemory(&bufferInitData, sizeof(bufferInitData));
			bufferInitData.pSysMem = getPoint ? verticies.data() : nullptr;

			ID3D11Buffer* ppBuffer;

			this->device->CreateBuffer(&bufferDesc, getPoint ? &bufferInitData : nullptr, &ppBuffer);

			VertexBuffer* buffer = new VertexBuffer(name, ppBuffer, numberOfElements, sizeof(T));

			buffer->Init(this->device);

			this->bufferController->AddBuffer(buffer);

			return S_OK;
		}

		template<class T> HRESULT CreateIndexBuffer(const std::wstring& name, UINT numberOfElements, std::function<HRESULT(UINT, T*)> getPoint)
		{
			vector<T> verticies;

			if (getPoint)
			{
				verticies = vector<T>(numberOfElements);

				T simpleVertex; ZeroMemory(&simpleVertex, sizeof(T));

				UINT index = 0;
				while (index < numberOfElements && getPoint(index, &simpleVertex) == S_OK)
				{
					verticies[index++] = simpleVertex;
				}
			}

			D3D11_BUFFER_DESC bufferDesc; ZeroMemory(&bufferDesc, sizeof(bufferDesc));
			bufferDesc.ByteWidth = numberOfElements * sizeof(T);
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = sizeof(T);

			D3D11_SUBRESOURCE_DATA bufferInitData;
			ZeroMemory(&bufferInitData, sizeof(bufferInitData));
			bufferInitData.pSysMem = getPoint ? verticies.data() : nullptr;

			ID3D11Buffer* ppBuffer;

			this->device->CreateBuffer(&bufferDesc, getPoint ? &bufferInitData : nullptr, &ppBuffer);

			IndexBuffer* buffer = new IndexBuffer(name, ppBuffer, numberOfElements, sizeof(T));

			buffer->Init(this->device);

			this->bufferController->AddBuffer(buffer);

			return S_OK;
		}

		HRESULT CreateControllers();

		HRESULT SetTopology(D3D_PRIMITIVE_TOPOLOGY setting)
		{
			this->immediateContext->IASetPrimitiveTopology(setting);

			return S_OK;
		}

		HRESULT InitControllers();

		PickingData TestPixel(XMINT2);

		HRESULT LoadTextures();
	};
}