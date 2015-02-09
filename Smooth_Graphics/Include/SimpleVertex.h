#pragma once

#include <directxmath.h>

namespace SmoothGraphics
{
	struct SimpleVertex
	{
		DirectX::XMFLOAT4 Pos;
		DirectX::XMFLOAT4 Colour;
		DirectX::XMFLOAT4 Vel;

		SimpleVertex(float x, float y, float z, float r, float g, float b);

		SimpleVertex();
	};

	struct SimpleRenderVertex
	{
		DirectX::XMFLOAT2 Pos;
		DirectX::XMFLOAT2 Tex;

		SimpleRenderVertex(float x, float y, float tx, float ty)
		{
			this->Pos.x = x;
			this->Pos.y = y;
			this->Tex.x = tx;
			this->Tex.y = ty;
		}

		SimpleRenderVertex()
		{

		}
	};

	struct MovingParticleData
	{
		DirectX::XMFLOAT3 Accel;
		DirectX::XMFLOAT3 Target;
		DirectX::XMFLOAT4 TargetColour;
	};
}