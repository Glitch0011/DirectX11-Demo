#include <ShaderController.h>
#include <fstream>
#include <FileSystem.h>

#include <WinBase.h>

#include <d3dcompiler.h>
#include <directxmath.h>

using namespace std;
using namespace SmoothGraphics;
using namespace SmoothHardware;

vector<char>* ShaderController::LoadShaderData(const std::wstring& file)
{
	vector<char>* fileData = nullptr;

	ifstream fileStream(file, ios::in | ios::binary | ios::ate);

	if (fileStream.is_open())
	{
		int length = (int)fileStream.tellg();

		fileData = new vector<char>(length);

		fileStream.seekg(0, ios::beg);
		fileStream.read(fileData->data(), length);

		fileStream.close();
	}

	return fileData;
}

HRESULT ShaderController::LoadPixelShader(const std::wstring& file)
{
	auto data = this->LoadShaderData(file);

	ID3D11PixelShader* shader = nullptr;

	auto res = this->device->CreatePixelShader(data->data(), data->size(), nullptr, &shader);

	delete data;

	this->shaders.push_back(new PixelShader(Path(file).file_name(), shader));

	return res;
}

HRESULT ShaderController::LoadComputeShader(const std::wstring& file)
{
	auto data = this->LoadShaderData(file);

	ID3D11ComputeShader* shader = nullptr;

	auto res = this->device->CreateComputeShader(data->data(), data->size(), nullptr, &shader);

	ID3D11ShaderReflection* reflector = nullptr;

	D3DReflect(data->data(), data->size(), IID_ID3D11ShaderReflection, (void**)&reflector);

	if (res == S_OK)
	{
		this->shaders.push_back(new ComputeShader(Path(file).file_name(), shader, reflector));
	}
	else
	{
		auto test = L"----------------> Compute Shader (" + file + L") failed to compile\r\n";
		OutputDebugString(test.c_str());
	}
	
	delete data;

	return res;
}

HRESULT ShaderController::LoadGeometryShader(const std::wstring& file)
{
	auto data = this->LoadShaderData(file);

	ID3D11GeometryShader* shader = nullptr;

	auto res = this->device->CreateGeometryShader(data->data(), data->size(), nullptr, &shader);
	if (FAILED(res))
	{
		delete data;
		return res;
	}

	delete data;

	this->shaders.push_back(new GeometryShader(Path(file).file_name(), shader));

	return S_OK;
}

//http://takinginitiative.wordpress.com/2011/12/11/directx-1011-basic-shader-reflection-automatic-input-layout-creation/
HRESULT CreateInputLayoutDescFromVertexShaderSignature(std::vector<char>* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout)
{
	// Reflect shader info
	ID3D11ShaderReflection* pVertexShaderReflection = NULL;
	if (FAILED(D3DReflect(pShaderBlob->data(), pShaderBlob->size(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
	{
		return S_FALSE;
	}

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	pVertexShaderReflection->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (UINT i = 0; i< shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

		// fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// determine DXGI format
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		//save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	// Try to create Input Layout
	HRESULT hr = pD3DDevice->CreateInputLayout(&inputLayoutDesc[0], inputLayoutDesc.size(), pShaderBlob->data(), pShaderBlob->size(), pInputLayout);

	//Free allocation shader reflection memory
	pVertexShaderReflection->Release();
	return hr;
}

HRESULT ShaderController::LoadVertexShaderFile(const std::wstring& file)
{
	auto data = this->LoadShaderData(file);

	ID3D11VertexShader* shader = nullptr;

	auto res = this->device->CreateVertexShader(data->data(), data->size(), nullptr, &shader);
	if (FAILED(res))
	{
		delete data;
		return res;
	}

	ID3D11InputLayout* layout = nullptr;
	res = CreateInputLayoutDescFromVertexShaderSignature(data, this->device, &layout);

	/*D3D11_INPUT_ELEMENT_DESC layoutDescription[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "COLOR", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ID3D11InputLayout* layout = nullptr;
	
	res = this->device->CreateInputLayout(layoutDescription, ARRAYSIZE(layoutDescription), data->data(), data->size(), &layout);*/

	delete data;

	this->shaders.push_back(new VertexShader(Path(file).file_name(), shader, layout));

	return res;
}

HRESULT ShaderController::LoadAllShaders(const std::wstring& _path)
{
	DirectoryIteractor end_itr;

	for (DirectoryIteractor itr(_path); itr != end_itr; ++itr)
	{
		if (itr->has_extension())
		{
			if (itr->extension() == L"p_cso")
			{
				this->LoadPixelShader(*itr);
			}
			else if (itr->extension() == L"v_cso")
			{
				this->LoadVertexShaderFile(*itr);
			}
			else if (itr->extension() == L"g_cso")
			{
				this->LoadGeometryShader(*itr);
			}
			else if (itr->extension() == L"c_cso")
			{
				this->LoadComputeShader(*itr);
			}
		}	
	}

	return S_OK;
}