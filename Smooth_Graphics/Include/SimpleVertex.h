#pragma once

#include <directxmath.h>

namespace SmoothGraphics
{
	struct SimpleVertex
	{
		DirectX::XMFLOAT4 Pos;
		DirectX::XMFLOAT3 Rotation;
		DirectX::XMFLOAT3 Scale;
		DirectX::XMFLOAT4 Colour;
		DirectX::XMFLOAT4 Vel;

		SimpleVertex(float x, float y, float z, float r, float g, float b);

		SimpleVertex();
	};

	struct MovingParticleData
	{
		DirectX::XMFLOAT3 Accel;
		DirectX::XMFLOAT3 Target;
		DirectX::XMFLOAT4 TargetColour;
		DirectX::XMFLOAT3 TargetRotation;
		DirectX::XMFLOAT3 TargetScale;
	};
}