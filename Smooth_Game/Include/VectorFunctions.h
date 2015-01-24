#include <DirectXMath.h>

using namespace DirectX;

static XMFLOAT3 Add(XMFLOAT3 l, XMFLOAT3 r)
{
	return XMFLOAT3(l.x + r.x, l.y + r.y, l.z + r.z);
}

static XMFLOAT3 Multiply(XMFLOAT3 l, XMFLOAT3 r)
{
	return XMFLOAT3(l.x * r.x, l.y * r.y, l.z * r.z);
}

static XMFLOAT3 Subtract(XMFLOAT3 l, XMFLOAT3 r)
{
	return XMFLOAT3(l.x - r.x, l.y - r.y, l.z - r.z);
}