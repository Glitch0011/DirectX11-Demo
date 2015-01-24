#pragma once

#include <BillboardVertexBuffer.h>
#include <DirectXHelper.h>
#include <vector>

using namespace std;
using namespace SmoothGraphics;
using namespace DirectX;

BillboardVertexBuffer::BillboardVertexBuffer(wstring name, function<HRESULT(int, SimpleVertex*)> getPoint, int count) : VertexBuffer(name, nullptr, elementCount, 0)
{
	this->getPoint = getPoint;
}

D3D11_BUFFER_DESC BillboardVertexBuffer::GetDescription()
{
	D3D11_BUFFER_DESC descBuf; ZeroMemory(&descBuf, sizeof(descBuf));
	this->Get()->GetDesc(&descBuf);
	return descBuf;
}

HRESULT BillboardVertexBuffer::CreateBufferResourceView(ID3D11Device* device)
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC viewDescription;
	ZeroMemory(&viewDescription, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

	viewDescription.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	viewDescription.Buffer.FirstElement = 0;

	auto description = this->GetDescription();
	if (description.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS) //Raw buffer
	{
		viewDescription.Format = DXGI_FORMAT_R32_TYPELESS;
		viewDescription.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		viewDescription.Buffer.NumElements = description.ByteWidth / sizeof(SimpleVertex);
	}
	else if (this->GetDescription().MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) //Structured Buffer
	{
		viewDescription.Format = DXGI_FORMAT_UNKNOWN;
		viewDescription.Buffer.NumElements = description.ByteWidth / description.StructureByteStride;
	}
	else
	{
		return E_INVALIDARG;
	}

	return device->CreateUnorderedAccessView(this->Get(), &viewDescription, &this->uavResourceView);
}

HRESULT BillboardVertexBuffer::Init(ID3D11Device* device)
{
	std::vector<SimpleVertex> vertices = std::vector<SimpleVertex>(this->elementCount);

	SimpleVertex simpleVertex; ZeroMemory(&simpleVertex, sizeof(SimpleVertex));

	UINT index = 0;
	while (getPoint(index, &simpleVertex) == S_OK && index < this->elementCount)
	{
		vertices[index++] = simpleVertex;
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	
	bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bd.ByteWidth = sizeof(SimpleVertex) * vertices.size();
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride = sizeof(SimpleVertex);
	bd.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));

	InitData.pSysMem = vertices.data();

	auto res = device->CreateBuffer(&bd, &InitData, &this->buffer);
	SetDebugObjectName(this->buffer, this->name);

	if (FAILED(res))
	{
		return res;
	}

	this->elementCount = vertices.size();

	res = this->CreateUAV(device);
	if (FAILED(res))
		return res;
	return this->CreateSRV(device);
}