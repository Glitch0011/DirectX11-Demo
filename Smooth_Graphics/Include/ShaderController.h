#pragma once

#include <d3d11.h>
#include <vector>
#include <Shader.h>
#include <concurrent_vector.h>

namespace SmoothGraphics
{
	class ShaderController
	{
		concurrency::concurrent_vector<Shader*> shaders;
		ID3D11Device* device = nullptr;

	public:
		ShaderController(ID3D11Device* device)
		{
			this->device = device;
		}

		~ShaderController()
		{
			for (auto shader : this->shaders)
			{
				shader->Release();
				delete shader;
			}
		}

		HRESULT Init()
		{
			HRESULT hr = S_OK;

			if (FAILED(hr = this->LoadAllShaders(L"Shaders")))
				return hr;

			return S_OK;
		}

		std::vector<char>* LoadShaderData(const std::wstring& file);

		HRESULT LoadPixelShader(const std::wstring& file);

		HRESULT LoadVertexShaderFile(const std::wstring& file);

		HRESULT LoadGeometryShader(const std::wstring& file);

		HRESULT LoadComputeShader(const std::wstring& file);

		HRESULT LoadAllShaders(const std::wstring& path);

		Shader* operator[](const std::wstring& name)
		{
			for (auto shader : shaders)
			{
				if (shader->name == name)
				{
					return shader;
				}
			}

			return nullptr;
		}

		Shader* operator[](int name)
		{
			return this->shaders[name];
		}

		Shader* Get(const std::wstring& name)
		{
			return this->operator[](name);
		}
	};
}