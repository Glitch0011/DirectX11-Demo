#include <TextureBuffer.h>
#include <DirectXHelper.h>

namespace SmoothGraphics
{
	class DepthTextureBuffer : public TextureBuffer
	{
	public:
		ID3D11DepthStencilView* depthStencilView;

		DepthTextureBuffer(std::wstring name, ID3D11Resource* buffer) :
			TextureBuffer(name, buffer)
		{

		}

		virtual HRESULT Init(ID3D11Device* device)
		{
			// Create the depth stencil view
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV; ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

			descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice = 0;

			HRESULT err;
			if (FAILED(err = device->CreateDepthStencilView(this->Get(), &descDSV, &depthStencilView)))
				return err;

			SetDebugObjectName(this->depthStencilView, this->name + L"_DSV");

			return TextureBuffer::Init(device);
		}
	};
}