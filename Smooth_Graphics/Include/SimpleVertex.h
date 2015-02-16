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

	struct PlayerData
	{
		DirectX::XMFLOAT3 Pos;
		float index;
		float followRadius;
		DirectX::XMFLOAT3 Padding;
	};

	struct MovingParticleData
	{
		DirectX::XMFLOAT4 TargetColour;
		DirectX::XMFLOAT3 Target;
		float State;
		float Player;
		float Padding1;
		float Padding2;
		float Padding3;
	};
}