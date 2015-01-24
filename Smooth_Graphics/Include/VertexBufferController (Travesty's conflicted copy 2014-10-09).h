#include <d3d11.h>
#include <vector>
#include <SimpleVertex.h>
#include <string>
#include <NamedResource.h>
#include <BufferController.h>

using namespace std;

namespace SmoothGraphics
{
	class VertexBuffer : public Buffer
	{
	protected:
		ID3D11UnorderedAccessView* resourceView;

	public:
		VertexBuffer(wstring name) : 
			Buffer(name, nullptr)
		{
			this->buffer = nullptr;
			this->resourceView = nullptr;
		}

		virtual HRESULT Init(ID3D11Device* device) = 0;

		virtual HRESULT Set(ID3D11DeviceContext*) = 0;

		virtual HRESULT Release() = 0;

		ID3D11Buffer* Get()
		{
			return this->buffer;
		}

		ID3D11UnorderedAccessView* ResourceView()
		{
			return this->resourceView;
		}
	};

	class TriangleVertexBuffer :
		public VertexBuffer
	{
	public:
		vector<SimpleVertex> vertices;

		TriangleVertexBuffer(wstring name) :
			VertexBuffer(name)
		{
			float size = 5;
			vertices = vector<SimpleVertex>(size * size * size);

			int i = 0;
			float spacing = 1.0f;
			for (int x = 0; x < size; x++)
			{
				for (int y = 0; y < size; y++)
				{
					for (int z = 0; z < size; z++)
					{
						auto c_x = (1.0f / (float)size) * x;
						auto c_y = (1.0f / (float)size) * y;
						auto c_z = (1.0f / (float)size) * z;
						vertices[i++] = SimpleVertex(x * spacing, y * spacing, z * spacing, c_x, c_y, c_z);
					}
				}
			}
		}

		D3D11_BUFFER_DESC GetDescription()
		{
			D3D11_BUFFER_DESC descBuf; ZeroMemory(&descBuf, sizeof(descBuf));
			this->Get()->GetDesc(&descBuf);
			return descBuf;
		}

		HRESULT CreateBufferResourceView(ID3D11Device* device)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC viewDescription;
			ZeroMemory(&viewDescription, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
			viewDescription.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			viewDescription.Buffer.FirstElement = 0;

			auto description = this->GetDescription();
			if (description.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
			{
				// This is a Raw Buffer
				viewDescription.Format = DXGI_FORMAT_R32_TYPELESS;
				viewDescription.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
				viewDescription.Buffer.NumElements = description.ByteWidth / 4;
			}
			else if (this->GetDescription().MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
			{
				// This is a Structured Buffer
				viewDescription.Format = DXGI_FORMAT_UNKNOWN;
				viewDescription.Buffer.NumElements = description.ByteWidth / description.StructureByteStride;
			}
			else
			{
				return E_INVALIDARG;
			}

			return device->CreateUnorderedAccessView(this->Get(), &viewDescription, &this->resourceView);
		}

		HRESULT Init(ID3D11Device* device)
		{
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(SimpleVertex) * this->vertices.size();
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;
			bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
			bd.StructureByteStride = sizeof(SimpleVertex);
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA InitData;
			ZeroMemory(&InitData, sizeof(InitData));
			InitData.pSysMem = vertices.data();

			auto res = device->CreateBuffer(&bd, &InitData, &this->buffer);

			if (FAILED(res))
			{
				return res;
			}

			return this->CreateBufferResourceView(device);
		}

		HRESULT Set(ID3D11DeviceContext* immediateContext)
		{
			UINT stride = sizeof(SimpleVertex);
			UINT offset = 0;

			immediateContext->IASetVertexBuffers(0, 1, &this->buffer, &stride, &offset);

			return S_OK;
		}

		HRESULT Release()
		{
			this->buffer->Release();

			return S_OK;
		}
	};

	class VertexBufferController
	{
		ID3D11Device* device;
		BufferController* bufferController;

	public:
		VertexBufferController(ID3D11Device* device, BufferController* bufferController)
		{
			this->device = device;
			this->bufferController = bufferController;
		}

		~VertexBufferController()
		{

		}

		HRESULT Init()
		{
			return S_OK;
		}

		void AddVertexBuffer(VertexBuffer* buffer)
		{
			buffer->Init(this->device);
			this->bufferController->AddBuffer(buffer);
		}

		VertexBuffer* operator[](const wstring& name)
		{
			return dynamic_cast<VertexBuffer*>(this->bufferController->Get(name));
		}

		VertexBuffer* Get(const wstring& name)
		{
			return this->operator[](name);
		}
	};
}