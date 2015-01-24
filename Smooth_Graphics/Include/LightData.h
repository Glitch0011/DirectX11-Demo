#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

struct LightData
{
	XMFLOAT3 Pos;
	XMFLOAT3 Diffuse;
	FLOAT  Range;
	XMFLOAT3 Attr;
	FLOAT Angle;
	XMFLOAT3 Colour;
	UINT Type;
	XMFLOAT3 Direction;
};