#pragma once

#include <Renderer.h>
#include <BillboardVertexBuffer.h>
#include <ConstantBuffer.h>
#include <SimpleVertex.h>
#include <PositionData.h>

using namespace SmoothGraphics;

namespace SmoothGame
{
	class BillboardRendererComponent :
		public Renderer
	{
	public:
		volatile int size;
		PositionalData* objData;

		BillboardRendererComponent(int size);

		HRESULT Init();
	};
}