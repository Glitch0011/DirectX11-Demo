#pragma once

#include <GraphicsEngine.h>
#include <Component.h>
#include <ConstantBuffer.h>
#include <BasicConstantBuffer.h>

using namespace SmoothGraphics;

namespace SmoothGame
{
	class Renderer : public Component
	{
	protected:
		SmoothGraphics::GraphicsEngine* graphics;

		Renderer();

		virtual ~Renderer();

		virtual HRESULT Init();
	};
}