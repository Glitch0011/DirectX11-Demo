#include <SimpleVertex.h>

using namespace SmoothGraphics;
using namespace DirectX;

SimpleVertex::SimpleVertex(float x, float y, float z, float r, float g, float b)
{
	this->Pos = XMFLOAT4(x, y, z, 1.0f);
	this->Colour = XMFLOAT4(r, g, b, 1);
}

SimpleVertex::SimpleVertex()
{
	this->Pos = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	this->Colour = XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
}