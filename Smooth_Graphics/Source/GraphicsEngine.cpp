#include <GraphicsEngine.h>

#include <random>
#include <time.h>
#include <future>

#include <iostream>
#include <exception>

#include <PngFile.h>

using namespace SmoothGraphics;
using namespace SmoothHardware;
using namespace std;

struct QuadConstantBuffer
{
	XMINT2 screenSize;
	UINT lightCount;
	float padding;
};

HRESULT GraphicsEngine::InitDevice(HWND g_hWnd)
{
	this->hWnd = g_hWnd;

	this->onError = [=](const wstring& msg, HRESULT res)
	{
		if (FAILED(res))
		{
			throw 1;
		}
	};

	this->onError(L"Error", this->CreateDeviceAndSwapChain());
	this->onError(L"Error", this->CreateControllers());

	colourRenderTarget = CachedVariable<TextureBuffer>(TO_FUNCTION(this->GetResource<TextureBuffer>(L"colourBuffer")));
	normalRenderTarget = CachedVariable<TextureBuffer>(TO_FUNCTION(this->GetResource<TextureBuffer>(L"normalBuffer")));
	depthRenderTarget = CachedVariable<TextureBuffer>(TO_FUNCTION(this->GetResource<TextureBuffer>(L"depthBuffer")));
	pickingRenderTarget = CachedVariable<TextureBuffer>(TO_FUNCTION(this->GetResource<TextureBuffer>(L"pickingBuffer")));
	stencilBuffer = CachedVariable<DepthTextureBuffer>(TO_FUNCTION(this->GetResource<DepthTextureBuffer>(L"stencilBuffer")));
	lightData = CachedVariable<StructuredBuffer>(TO_FUNCTION(this->Get<StructuredBuffer>(L"lightData")));
	quadVertexBuffer = CachedVariable<VertexBuffer>(TO_FUNCTION(this->Get<VertexBuffer>(L"quad_vertex")));
	quadIndexBuffer = CachedVariable<IndexBuffer>(TO_FUNCTION(this->Get<IndexBuffer>(L"quad_index")));
	quadBasicConstantBuffer = CachedVariable<ConstantBuffer>(TO_FUNCTION(this->Get<ConstantBuffer>(L"BasicConstantBuffer")));
	quadSamplerState = CachedVariable<ID3D11SamplerState>(TO_FUNCTION(this->GetSamplerState(L"samplerState")));

	vector<std::future<void>> futures;

	futures.push_back(std::async(launch::async, [=](){this->onError(L"Error", this->CreateRenderTargetView()); }));
	futures.push_back(std::async(launch::async, [=](){this->onError(L"Error", this->CreateViewport()); }));
	futures.push_back(std::async(launch::async, [=](){this->onError(L"Error", this->CreateDepthStencilState()); }));
	futures.push_back(std::async(launch::async, [=](){this->onError(L"Error", this->CreateRasterState()); }));
	futures.push_back(std::async(launch::async, [=](){this->onError(L"Error", this->CreateColourTexture()); }));
	futures.push_back(std::async(launch::async, [=](){this->onError(L"Error", this->CreateNormalTexture()); }));
	futures.push_back(std::async(launch::async, [=](){this->onError(L"Error", this->CreateDepthTexture()); }));
	futures.push_back(std::async(launch::async, [=](){this->onError(L"Error", this->CreatePickingTexture()); }));
	futures.push_back(std::async(launch::async, [=](){this->onError(L"Error", this->CreateStencilTexture()); }));
	futures.push_back(std::async(launch::async, [=](){this->onError(L"Error", this->CreateSamplerState()); }));
	futures.push_back(std::async(launch::async, [=](){this->onError(L"Error", this->CreatePickingStagingTexture()); }));

	for (auto& i : futures)
		i.wait();

	async(launch::async, [=]()
	{
		vector<std::future<void>> futures;

		futures.push_back(std::async(launch::async, [=](){this->onError(L"Error", this->InitControllers()); }));

		futures.push_back(std::async(launch::async, [=]()
		{
			//Setup quad vertex
			this->CreateVertexBuffer<XMFLOAT3>(L"quad_vertex", 4, [=](UINT index, XMFLOAT3* vertex)
			{
				XMFLOAT3 v[] {
					XMFLOAT3(-1.0f, -1.0f, 0.5f),
					XMFLOAT3(-1.0f, 1.0f, 0.5f),
					XMFLOAT3(1.0f, 1.0f, 0.5f),
					XMFLOAT3(1.0f, -1.0f, 0.5f),
				};
				*vertex = v[index];
				return S_OK;
			});
		}));

		futures.push_back(std::async(launch::async, [=]()
		{
			//Create quad index buffer
			this->CreateIndexBuffer<int>(L"quad_index", 6, [=](UINT index, int* vertex)
			{
				int v[] {
					0, 1, 2,
					3, 1, 0,
				};
				*vertex = v[index];
				return S_OK;
			});
		}));

		futures.push_back(std::async(launch::async, [=]()
		{
			this->bufferController->CreateConstantBuffer<QuadConstantBuffer>(L"quadConstantBuffer");
		}));

		futures.push_back(std::async(launch::async, [=]()
		{
			this->LoadTextures();
		}));

		for (auto& i : futures)
			i.wait();
	});
	
	if (this->onCreateScene)
		this->onCreateScene();

	return S_OK;
}

PickingData GraphicsEngine::TestPixel(XMINT2 screenPos)
{
	D3D11_BOX box;
	ZeroMemory(&box, sizeof(D3D11_BOX));

	box.left = screenPos.x;
	box.right = screenPos.x + 1;
	box.top = screenPos.y;
	box.bottom = screenPos.y + 1;
	box.back = 1;
	box.front = 0;

	auto a = this->bufferController->GetResource(L"pickingResponceTexture");

	this->immediateContext->CopySubresourceRegion(
		a->Get(),
		0,
		0,
		0,
		0,
		pickingRenderTarget->Get(),
		0,
		&box);

	PickingData data;

	{
		auto m = MappedGpuMemory(this->immediateContext, this->bufferController->GetResource(L"pickingResponceTexture")->Get(), D3D11_MAP_READ);
		memcpy(&data, m.Data(), sizeof(PickingData));
	}

	return data;
}

HRESULT GraphicsEngine::LoadTextures()
{
	DirectoryIteractor end_itr;
	wstring path = L"./Assets";

	for (DirectoryIteractor itr(path); itr != end_itr; ++itr)
	{
		if (itr->has_extension())
		{
			if (itr->extension() == L"bmp")
			{
				auto name = itr->file_name();

				BmpFile bmpFile(*itr);

				D3D11_TEXTURE2D_DESC descDepth;
				descDepth.Width = bmpFile.width;
				descDepth.Height = bmpFile.height;
				descDepth.MipLevels = 1;
				descDepth.ArraySize = 1;

				descDepth.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				descDepth.SampleDesc.Count = 1;
				descDepth.SampleDesc.Quality = 0;
				descDepth.Usage = D3D11_USAGE_DEFAULT;
				descDepth.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				descDepth.CPUAccessFlags = 0;
				descDepth.MiscFlags = 0;

				HRESULT hr;
				if (FAILED(hr = this->CreateTexture(name, descDepth, bmpFile.data)))
					return S_OK;
			}
			else if (itr->extension() == L"png")
			{
				auto name = itr->file_name();

				PngFile pngFile(*itr);

				D3D11_TEXTURE2D_DESC descDepth;
				descDepth.Width = pngFile.width;
				descDepth.Height = pngFile.height;
				descDepth.MipLevels = 1;
				descDepth.ArraySize = 1;

				descDepth.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				descDepth.SampleDesc.Count = 1;
				descDepth.SampleDesc.Quality = 0;
				descDepth.Usage = D3D11_USAGE_DEFAULT;
				descDepth.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				descDepth.CPUAccessFlags = 0;
				descDepth.MiscFlags = 0;

				HRESULT hr;
				if (FAILED(hr = this->CreateTexture(name, descDepth, pngFile.data)))
					return S_OK;
			}
		}
	}

	return S_OK;
}

HRESULT GraphicsEngine::CreatePickingStagingTexture()
{
	//Create picking recieve texture
	D3D11_TEXTURE2D_DESC dts;
	dts.Width = 1;
	dts.Height = 1;
	dts.MipLevels = 1;
	dts.ArraySize = 1;

	dts.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	dts.SampleDesc.Count = 1;
	dts.SampleDesc.Quality = 0;
	dts.Usage = D3D11_USAGE_STAGING;
	dts.BindFlags = 0;// D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	dts.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	dts.MiscFlags = 0;

	return this->CreateTexture(L"pickingResponceTexture", dts);
}

HRESULT GraphicsEngine::CreateStencilTexture()
{
	auto size = this->GetSize();

	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = size.x;
	descDepth.Height = size.y;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;

	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	return this->CreateTexture<DepthTextureBuffer>(L"stencilBuffer", descDepth);
}

HRESULT GraphicsEngine::CreateSamplerState()
{
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = 0;

	ID3D11SamplerState* samplerState;

	HRESULT hr;
	if (FAILED(hr = this->device->CreateSamplerState(&sampDesc, &samplerState)))
		return hr;

	this->samplerStates[L"samplerState"] = samplerState;

	return S_OK;
}

HRESULT GraphicsEngine::SetRenderTargetToBackBuffer()
{
	this->immediateContext->OMSetDepthStencilState(nullptr, 1);
	this->immediateContext->OMSetRenderTargets(1, &this->renderTargetView, nullptr);

	return S_OK;
};

HRESULT GraphicsEngine::SetDeferredRenderTargets()
{
	/*auto colourRenderTarget = this->GetResource<TextureBuffer>(L"colourBuffer");
	auto normalRenderTarget = this->GetResource<TextureBuffer>(L"normalBuffer");
	auto depthRenderTarget = this->GetResource<TextureBuffer>(L"depthBuffer");*/

	if (colourRenderTarget && normalRenderTarget && depthRenderTarget && pickingRenderTarget)
	{
		ID3D11RenderTargetView* views[] = {
			colourRenderTarget->renderTargetView,
			normalRenderTarget->renderTargetView,
			depthRenderTarget->renderTargetView,
			pickingRenderTarget->renderTargetView,
		};

		auto stencilTextureBuffer = this->GetResource<DepthTextureBuffer>(L"stencilBuffer");

		this->immediateContext->OMSetRenderTargets(sizeof(views) / sizeof(void*), views, stencilTextureBuffer ? stencilTextureBuffer->depthStencilView : nullptr);
	}

	return S_OK;
}

HRESULT GraphicsEngine::CleanupDevice()
{
	if (immediateContext)
		immediateContext->ClearState();
	if (renderTargetView)
		renderTargetView->Release();
	if (swapChain)
		swapChain->Release();
	if (immediateContext)
		immediateContext->Release();
	if (device)
		device->Release();

	delete this->shaderController, this->vertexBufferController, this->modelController;

	return S_OK;
}

HRESULT GraphicsEngine::Render()
{
	float ClearColor[4] = { 0.001f, 0.001f, 0.001f, 1.0f };
	float ClearNormal[4] = { 0.000f, 0.000f, 0.000f, 0.000f };

	/*IF_NOT_NULL(this->renderTargetView, immediateContext->ClearRenderTargetView(this->renderTargetView, ClearColor));
	IF_NOT_NULL(this->colourRenderTarget, immediateContext->ClearRenderTargetView(this->colourRenderTarget->renderTargetView, ClearColor));
	IF_NOT_NULL(this->normalRenderTarget, immediateContext->ClearRenderTargetView(this->normalRenderTarget->renderTargetView, ClearNormal));
	IF_NOT_NULL(this->depthRenderTarget, immediateContext->ClearRenderTargetView(this->depthRenderTarget->renderTargetView, ClearNormal));
	IF_NOT_NULL(this->stencilBuffer, immediateContext->ClearDepthStencilView(this->stencilBuffer->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0));*/

	//this->SetDeferredRenderTargets();

	this->SetRenderTargetToBackBuffer();

	immediateContext->ClearRenderTargetView(this->renderTargetView, ClearColor);

	if (this->onRender)
		this->onRender();

	//this->DrawDeferredQuad();

	swapChain->Present(1, 0);

	return S_OK;
}

HRESULT GraphicsEngine::DrawDeferredQuad()
{
	this->SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	auto pixelShader = this->GetShader<PixelShader>(L"Quad_PS");
	auto vertexShader = this->GetShader<VertexShader>(L"Quad_VS");
	auto quadConstantBuffer = this->Get<ConstantBuffer>(L"quadConstantBuffer");

	if (vertexShader && pixelShader && quadConstantBuffer)
	{
		QuadConstantBuffer buffer;
		buffer.lightCount = this->lightController->getCount();
		buffer.screenSize = XMINT2(this->width, this->height);
		quadConstantBuffer->Update(this->immediateContext, &buffer);

		IF_NOT_NULL(vertexShader, vertexShader->SetInputLayout(this->immediateContext));
		IF_NOT_NULL(vertexShader, vertexShader->Set(this->immediateContext));
		IF_NOT_NULL(pixelShader, pixelShader->Set(this->immediateContext));

		IF_NOT_NULL(quadVertexBuffer, quadVertexBuffer->Set(this->immediateContext));
		IF_NOT_NULL(quadIndexBuffer, quadIndexBuffer->Set(this->immediateContext));

		//Make sure the light buffers are up to date
		this->lightController->UpdateBuffers(this->immediateContext);

		IF_NOT_NULL(colourRenderTarget,  this->immediateContext->PSSetShaderResources(0, 1, &colourRenderTarget->srvResourceView));
		IF_NOT_NULL(normalRenderTarget,  this->immediateContext->PSSetShaderResources(1, 1, &normalRenderTarget->srvResourceView));
		IF_NOT_NULL(depthRenderTarget,   this->immediateContext->PSSetShaderResources(2, 1, &depthRenderTarget->srvResourceView));
		//IF_NOT_NULL(pickingRenderTarget, this->immediateContext->PSSetShaderResources(3, 1, &pickingRenderTarget->srvResourceView));
		IF_NOT_NULL(lightData,           this->immediateContext->PSSetShaderResources(3, 1, &lightData->srvResourceView));

		IF_NOT_NULL(quadConstantBuffer, this->immediateContext->PSSetConstantBuffers(0, 1, &quadConstantBuffer->buffer));
		IF_NOT_NULL(quadBasicConstantBuffer, this->immediateContext->PSSetConstantBuffers(1, 1, &quadBasicConstantBuffer->buffer));

		IF_NOT_NULL(quadSamplerState, this->immediateContext->PSSetSamplers(0, 1, quadSamplerState.GetPtr()));

		IF_NOT_NULL(quadIndexBuffer, quadIndexBuffer->Draw(this->immediateContext));
	}

	this->UnsetVertexBuffer();

	ID3D11ShaderResourceView* blank = nullptr;
	ID3D11Buffer* blankBuffer = nullptr;

	this->immediateContext->PSSetShaderResources(0, 1, &blank);
	this->immediateContext->PSSetShaderResources(1, 1, &blank);
	this->immediateContext->PSSetShaderResources(2, 1, &blank);
	this->immediateContext->PSSetShaderResources(3, 1, &blank);
	this->immediateContext->PSSetConstantBuffers(0, 1, &blankBuffer);
	this->immediateContext->PSSetConstantBuffers(1, 1, &blankBuffer);

	return S_OK;
}

HRESULT GraphicsEngine::CreateDeviceAndSwapChain()
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;

	this->CalculateWidthAndHeight();

	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	HRESULT hr = S_OK;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		driverType = driverTypes[driverTypeIndex];

		hr = D3D11CreateDeviceAndSwapChain(NULL,
			driverType,
			NULL,
			createDeviceFlags,
			featureLevels,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&sd,
			&swapChain,
			&device,
			&featureLevel,
			&immediateContext);

		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	if (this->featureLevel < D3D_FEATURE_LEVEL_11_0)
	{
		D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hardwareOptions = { 0 };
		this->device->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &hardwareOptions, sizeof(hardwareOptions));

		if (!hardwareOptions.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
		{
			std::wstring test = L"----------------> DirectCompute is not supported by this device\r\n";
			OutputDebugString(test.c_str());
		}
	}

	return hr;
}

HRESULT GraphicsEngine::CreateRenderTargetView()
{
	ID3D11Texture2D* backBuffer = nullptr;

	HRESULT hr;

	if (FAILED(hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer)))
		return hr;

	SetDebugObjectName(backBuffer, L"BackBuffer");

	if (FAILED(hr = this->CreateRenderTargetView(backBuffer, &this->renderTargetView)))
		return hr;

	backBuffer->Release();

	return S_OK;
}

HRESULT GraphicsEngine::CreateViewport()
{
	D3D11_VIEWPORT vp;

	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	immediateContext->RSSetViewports(1, &vp);

	return S_OK;
}

HRESULT GraphicsEngine::CreateColourTexture()
{
	auto size = this->GetSize();

	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = size.x;
	descDepth.Height = size.y;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;

	descDepth.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	return this->CreateTexture(L"colourBuffer", descDepth);
}

HRESULT GraphicsEngine::CreateNormalTexture()
{
	auto size = this->GetSize();

	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = size.x;
	descDepth.Height = size.y;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;

	descDepth.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	return this->CreateTexture(L"normalBuffer", descDepth);
}

HRESULT GraphicsEngine::CreatePickingTexture()
{
	auto size = this->GetSize();

	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = size.x;
	descDepth.Height = size.y;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;

	descDepth.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	return this->CreateTexture(L"pickingBuffer", descDepth);
}

HRESULT GraphicsEngine::CreateDepthTexture()
{
	auto size = this->GetSize();

	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = size.x;
	descDepth.Height = size.y;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;

	descDepth.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	return this->CreateTexture(L"depthBuffer", descDepth);
}

HRESULT GraphicsEngine::CreateDepthStencilState()
{
	//Create Stencil State
	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.DepthEnable = true;

	// Stencil test parameters
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HRESULT err;
	if (FAILED(err = this->device->CreateDepthStencilState(&dsDesc, &depthStencilState)))
		return err;

	//immediateContext->OMSetDepthStencilState(this->depthStencilState, 1);

	return S_OK;
}

HRESULT GraphicsEngine::UnsetVertexBuffer()
{
	ID3D11Buffer* const buffer = nullptr;
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	this->immediateContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);

	return S_OK;
}

HRESULT GraphicsEngine::UnsetUnorderedAccessViews()
{
	ID3D11UnorderedAccessView* blankAccessView = nullptr;
	this->immediateContext->CSSetUnorderedAccessViews(0, 1, &blankAccessView, nullptr);

	return S_OK;
}

HRESULT GraphicsEngine::DispatchComputeShader(XMINT3 dimensions, Shader* shader)
{
	auto computeShader = dynamic_cast<ComputeShader*>(shader);

	if (computeShader)
	{
		//Attempt at having named parameters
		/*D3D11_SHADER_DESC desc; memset(&desc, 0, sizeof(D3D11_SHADER_DESC));

		computeShader->getReflection()->GetDesc(&desc);
		auto positionVariable = computeShader->getReflection()->GetVariableByName("positions");

		D3D11_SHADER_VARIABLE_DESC variableDescription; ZeroMemory(&variableDescription, sizeof(D3D11_SHADER_VARIABLE_DESC));
		positionVariable->GetDesc(&variableDescription);*/

		computeShader->Set(this->immediateContext);

		immediateContext->Dispatch(dimensions.x, dimensions.y, dimensions.z);

		return S_OK;
	}
	else
	{
		return E_INVALIDARG;
	}
}

HRESULT GraphicsEngine::Resize()
{
	if (this->immediateContext)
	{
		this->immediateContext->OMSetRenderTargets(0, 0, 0);

		if (this->renderTargetView)
			this->renderTargetView->Release();

		auto hr = swapChain->ResizeBuffers(0, this->width, this->height, DXGI_FORMAT_UNKNOWN, 0);

		if (FAILED(hr = this->CreateRenderTargetView()))
			return hr;

		if (FAILED(hr = this->CreateViewport()))
			return hr;

		/*if (FAILED(hr = this->CreateDepthStencilState()))
			return hr;

		if (FAILED(hr = this->CreateDepthStencilView()))
			return hr;*/

		if (FAILED(hr = this->SetDeferredRenderTargets()))
			return hr;

		return S_OK;
	}
	else
	{
		return E_INVALIDARG;
	}
}

HRESULT GraphicsEngine::CalculateWidthAndHeight()
{
	if (this->hWnd != nullptr)
	{
		RECT rc;
		GetClientRect(this->hWnd, &rc);
		this->height = rc.bottom - rc.top;
		this->width = rc.right - rc.left;
		return S_OK;
	}
	else
	{
		return E_INVALIDARG;
	}
}

HRESULT GraphicsEngine::Update(const double& timePassedInSeconds)
{
	if (this->onUpdate)
		this->onUpdate(timePassedInSeconds);

	return S_OK;
}

HRESULT GraphicsEngine::CreateRasterState()
{
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterDesc.AntialiasedLineEnable = false;
	//rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = false;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	auto err = this->device->CreateRasterizerState(&rasterDesc, &rasterState);

	if (FAILED(err))
		return err;

	this->immediateContext->RSSetState(rasterState);

	return S_OK;
}

HRESULT GraphicsEngine::CreateControllers()
{
	this->bufferController       = new BufferController(this->device);
	this->shaderController       = new ShaderController(this->device);
	this->vertexBufferController = new VertexBufferController(this->device, this->bufferController);
	this->modelController        = new ModelController();
	this->lightController        = new LightController(this->bufferController);

	return S_OK;
}

HRESULT GraphicsEngine::InitControllers()
{
	async(launch::async, [=](){this->onError(L"Error", this->bufferController->Init()); });
	async(launch::async, [=](){this->onError(L"Error", this->shaderController->Init()); });
	async(launch::async, [=](){this->onError(L"Error", this->vertexBufferController->Init()); });
	async(launch::async, [=](){this->onError(L"Error", this->modelController->Init()); });

	return S_OK;
}