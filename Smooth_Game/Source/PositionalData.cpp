#include <PositionData.h>

using namespace SmoothGame;

XMFLOAT3* PositionalData::Position()
{
	return &this->position;
}

XMFLOAT3* PositionalData::Scale()
{
	return &this->scale;
}

XMFLOAT3* PositionalData::Rotation()
{
	return &this->rotation;
}