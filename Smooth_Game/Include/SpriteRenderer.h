#pragma once

#include <Renderer.h>
#include <CachedVariable.h>
#include <string>
#include <PositionComponent.h>

namespace SmoothGame
{
	class SpriteRenderer : public Renderer
	{
	protected:
		std::wstring textureName;
			
		CachedVariable<TextureBuffer> texture;
		CachedVariable<ID3D11SamplerState> samplerState;
		CachedVariable<PositionalData> positionData;

		CachedVariable<VertexShader> vShader;
		CachedVariable<PixelShader> pShader;

		CachedVariable<ConstantBuffer> basicConstantBuffer;
		CachedVariable<DynamicConstantBuffer> quickConstantBuffer;

		CachedVariable<IndexBuffer> indexBuffer;
		CachedVariable<VertexBuffer> vertexBuffer;

	public:
		SpriteRenderer(std::wstring);
		virtual HRESULT Init();
	};
}