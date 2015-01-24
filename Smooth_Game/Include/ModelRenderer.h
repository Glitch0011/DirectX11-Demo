#pragma once

#include <string>
#include <Model.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <Renderer.h>
#include <PositionData.h>
#include <CachedVariable.h>
#include <Frustum.h>
#include <PickingData.h>

namespace SmoothGame
{
	class ModelRenderer : public Renderer
	{
	private:
		std::wstring modelName;
		
		Model* model;
		PositionalData* objData;
		
		std::wstring textureName;
		std::wstring pixelShaderName;
	public: 
		std::wstring environmentName = L"spheremap";
	private:
		std::wstring bumpName = L"flat";
		std::wstring geometryName = L"ModelGeometryShader";
	public:
		std::wstring heightMapName;
	private:
		CachedVariable<VertexShader> vertexShader;
		CachedVariable<PixelShader> pixelShader;
		CachedVariable<GeometryShader> geometryShader;
		CachedVariable<ID3D11SamplerState> samplerState;
		CachedVariable<TextureBuffer> texture, environmentTexture, bumpTexture, heightMapTexture;
		CachedVariable<Buffer> quickConstantBuffer, basicConstantBuffer, ambientConstantBuffer, highlightingDataConstantBuffer, textureAvaliableConstantBuffer;
		CachedVariable<Frustum> frustum;

	public:
		DirectX::XMFLOAT4 ambient;
		
		PickingData highlightData;

		ModelRenderer(std::wstring modelName, std::wstring textureName, float ambient = 0.0);

		void SetupVertexBuffers(Shape* shape);

		bool isRunning();

		HRESULT Init();

		bool ShouldDraw();

		void SetPixelShaderName(std::wstring name)
		{
			this->pixelShaderName = name;
			this->pixelShader.Clear();
		}

		void SetEnvironmentName(std::wstring name)
		{
			this->environmentName = name;
			this->environmentTexture.Clear();
		}
		
		void SetBumpName(std::wstring name)
		{
			this->bumpName = name;
			this->bumpTexture.Clear();
		}

		void SetGeometryShaderName(std::wstring name)
		{
			this->geometryName = name;
			this->geometryShader.Clear();
		}

		void SetHeightMap(std::wstring name)
		{
			this->heightMapName = name;
			this->heightMapTexture.Clear();
		}
	};
}