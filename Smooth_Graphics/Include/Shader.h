#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#include <string>
#include <NamedResource.h>
#include <DirectXHelper.h>

namespace SmoothGraphics
{
	class Shader : public NamedResource
	{
	public:
		Shader(std::wstring name) :
			NamedResource(name)
		{

		}

		virtual void* getProgram() = 0;
		virtual HRESULT Set(ID3D11DeviceContext*) = 0;
		virtual HRESULT Release() = 0;
	};

	class PixelShader : public Shader
	{
		ID3D11PixelShader* shader;

	public:
		PixelShader(std::wstring name, ID3D11PixelShader* shader) :
			Shader(name)
		{
			this->shader = shader;
			SetDebugObjectName(this->shader, name + L"_PS");
		}

		void* getProgram()
		{
			return this->shader;
		}

		HRESULT Set(ID3D11DeviceContext* context)
		{
			if (this->shader)
			{
				context->PSSetShader(this->shader, nullptr, 0);
				return S_OK;
			}
			else
			{
				return E_INVALIDARG;
			}
		}

		HRESULT Release()
		{
			this->shader->Release();
			return S_OK;
		}
	};

	class VertexShader : public Shader
	{
		ID3D11VertexShader* shader;
		ID3D11InputLayout* layout;

	public:
		VertexShader(std::wstring name, ID3D11VertexShader* shader, ID3D11InputLayout* layout) :
			Shader(name)
		{
			this->shader = shader;
			this->layout = layout;

			SetDebugObjectName(this->shader, name + L"_VS");
			SetDebugObjectName(this->layout, name + L"_IL");
		}

		void* getProgram()
		{
			return this->shader;
		}

		HRESULT SetInputLayout(ID3D11DeviceContext* deviceContext)
		{
			deviceContext->IASetInputLayout(this->layout);

			return S_OK;
		}

		ID3D11InputLayout* GetInputLayout()
		{
			return this->layout;
		}

		HRESULT Set(ID3D11DeviceContext* context)
		{
			if (this->shader)
			{
				context->VSSetShader(this->shader, nullptr, 0);
				return S_OK;
			}
			else
			{
				return E_INVALIDARG;
			}
		}

		HRESULT Release()
		{
			if (this->shader)
				this->shader->Release();
			return S_OK;
		}
	};

	class GeometryShader : public Shader
	{
		ID3D11GeometryShader* shader;

	public:
		GeometryShader(std::wstring name, ID3D11GeometryShader* shader) : 
			Shader(name)
		{
			this->shader = shader;
			SetDebugObjectName(this->shader, name + L"_GS");
		}

		void* getProgram()
		{
			return this->shader;
		}

		HRESULT Set(ID3D11DeviceContext* context)
		{
			if (this->shader)
			{
				context->GSSetShader(this->shader, nullptr, 0);
				return S_OK;
			}
			else
			{
				return E_INVALIDARG;
			}
		}

		HRESULT Release()
		{
			if (this->shader)
				this->shader->Release();
			return S_OK;
		}
	};

	class ComputeShader : public Shader
	{
		ID3D11ComputeShader* shader;
		ID3D11ShaderReflection* reflector;

	public:
		ComputeShader(std::wstring name, ID3D11ComputeShader* shader, ID3D11ShaderReflection* reflector) :
			Shader(name)
		{
			this->shader = shader;
			this->reflector = reflector;
			SetDebugObjectName(this->shader, name + L"_CS");
		}

		void* getProgram()
		{
			return this->shader;
		}

		HRESULT Set(ID3D11DeviceContext* context)
		{
			if (this->shader)
			{
				context->CSSetShader(this->shader, nullptr, 0);
				return S_OK;
			}
			else
			{
				return E_INVALIDARG;
			}
		}

		HRESULT Release()
		{
			if (this->shader)
				this->shader->Release();
			return S_OK;
		}

		ID3D11ShaderReflection* getReflection()
		{
			return reflector;
		}
	};
}