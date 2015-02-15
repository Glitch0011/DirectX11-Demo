#include <BufferController.h>

#include <ConstantBuffer.h>
#include <DirectXHelper.h>

using namespace SmoothGraphics;

BufferController::BufferController(ID3D11Device* device)
{
	this->resources.clear();
	this->buffers.clear();
	this->device = device;
}

BufferController::~BufferController()
{
	for (auto buffer : this->buffers)
		delete buffer.second;
}

Buffer* BufferController::Get(const std::wstring& name)
{
	if (this->buffers.find(name) != this->buffers.end())
		return this->buffers[name];
	else
		return nullptr;
}

Resource* BufferController::GetResource(const std::wstring& name)
{
	if (this->resources.find(name) != this->resources.end())
		return this->resources[name];
	else
		return nullptr;
}

void BufferController::AddResource(Resource* resource)
{
	this->resources[resource->name] = resource;
	SetDebugObjectName(resource->resource, resource->name);
}

void BufferController::AddBuffer(Buffer* buffer)
{
	this->buffers[buffer->name] = buffer;
	SetDebugObjectName(buffer->buffer, buffer->name);
}

HRESULT BufferController::Init()
{
	return S_OK;
}

HRESULT BufferController::CreateDynamicConstantBuffer(const std::wstring& name, D3D11_BUFFER_DESC constantBufferDesc)
{
	ID3D11Buffer* buffer = nullptr;

	auto hr = this->device->CreateBuffer(&constantBufferDesc, nullptr, &buffer);
	if (SUCCEEDED(hr))
	{
		SetDebugObjectName(buffer, name);
		this->AddBuffer(new DynamicConstantBuffer(name, buffer));
		return S_OK;
	}
	else
	{
		return hr;
	}
}

HRESULT BufferController::CreateConstantBuffer(const std::wstring& name, D3D11_BUFFER_DESC constantBufferDesc)
{
	ID3D11Buffer* buffer = nullptr;

	auto hr = this->device->CreateBuffer(&constantBufferDesc, nullptr, &buffer);
	if (SUCCEEDED(hr))
	{
		SetDebugObjectName(buffer, name);
		this->AddBuffer(new ConstantBuffer(name, buffer));
		return S_OK;
	}
	else
	{
		return hr;
	}
}