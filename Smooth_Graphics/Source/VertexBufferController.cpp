#include <VertexBufferController.h>

using namespace std;
using namespace SmoothGraphics;

VertexBufferController::VertexBufferController(ID3D11Device* device, BufferController* bufferController)
{
	this->device = device;
	this->bufferController = bufferController;
}

VertexBufferController::~VertexBufferController()
{

}

HRESULT VertexBufferController::Init()
{
	return S_OK;
}

void VertexBufferController::AddVertexBuffer(VertexBuffer* buffer)
{
	buffer->Init(this->device);
	this->bufferController->AddBuffer(buffer);
}

VertexBuffer* VertexBufferController::operator[](const wstring& name)
{
	return dynamic_cast<VertexBuffer*>(this->bufferController->Get(name));
}

VertexBuffer* VertexBufferController::Get(const wstring& name)
{
	return this->operator[](name);
}