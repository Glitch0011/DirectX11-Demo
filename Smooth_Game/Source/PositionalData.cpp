#include <PositionData.h>

using namespace SmoothGame;

XMFLOAT3* PositionalData::Position()
{
	return &this->position;
}

void PositionalData::Position(XMFLOAT3 data)
{
	this->position = data;
}

XMFLOAT3* PositionalData::Scale()
{
	return &this->scale;
}

void PositionalData::Scale(XMFLOAT3 data)
{
	this->scale = data;
}

XMFLOAT3* PositionalData::Rotation()
{
	return &this->rotation;
}