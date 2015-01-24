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

		HRESULT Update(ID3D11DeviceContext* context, void* data);
	};
}