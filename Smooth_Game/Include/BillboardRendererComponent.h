#pragma once

#include <SpriteRenderer.h>
#include <BillboardVertexBuffer.h>
#include <ConstantBuffer.h>
#include <SimpleVertex.h>
#include <PositionData.h>

using namespace SmoothGraphics;

namespace SmoothGame
{
	class BillboardRendererComponent :
		public SpriteRenderer
	{
	public:
		volatile int size;
		PositionalData* objData;

		BillboardRendererComponent(std::wstring texture, UINT32 size);

		CachedVariable<StructuredBuffer> billboardData;

		virtual HRESULT Init();
	};
}