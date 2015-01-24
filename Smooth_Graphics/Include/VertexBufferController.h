#include <d3d11.h>
#include <string>

#include <BufferController.h>
#include <VertexBuffer.h>

namespace SmoothGraphics
{
	class VertexBufferController
	{
		ID3D11Device* device;
		BufferController* bufferController;

	public:
		VertexBufferController(ID3D11Device* device, BufferController* bufferController);

		~VertexBufferController();

		HRESULT Init();

		void AddVertexBuffer(VertexBuffer* buffer);

		VertexBuffer* operator[](const std::wstring& name);

		VertexBuffer* Get(const std::wstring& name);
	};
}