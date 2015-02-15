#pragma once

#include <d3d11.h>
#include <string>
#include <Buffer.h>

namespace SmoothGraphics
{
	class ConstantBuffer : public Buffer
	{
	public:
		ConstantBuffer(std::wstring name, ID3D11Buffer* buffer);

		virtual HRESULT Update(ID3D11DeviceContext* context, void* data);
	};

	class DynamicConstantBuffer : public ConstantBuffer
	{
	public:
		DynamicConstantBuffer(std::wstring name, ID3D11Buffer* buffer);

		virtual HRESULT Update(ID3D11DeviceContext* context, void* data);
	};
}