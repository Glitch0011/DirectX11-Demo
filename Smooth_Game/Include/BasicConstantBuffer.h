#pragma once

#include <d3d11.h>
#include <SmoothGameTypeDefs.h>

namespace SmoothGraphics
{
	struct BasicConstantBuffer
	{
		XMMATRIX mView;
		XMMATRIX mProjection;
		XMVECTOR mCameraPosition;
	};

	struct QuickBasicConstantBuffer
	{
		XMMATRIX mWorld;
	};

	struct AmbientConstantBuffer
	{
		XMVECTOR ambient;
		SmoothGame::ID_TYPE modelID;
		bool highlight;
	};

	struct TextureAvaliabilityConstantBuffer
	{
		float textureMapped;
		float bumpMapped;
		float environmentMapped;
		float heightMapped;
	};
}