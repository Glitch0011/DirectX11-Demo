#pragma once

#include <DirectXMath.h>

using namespace DirectX;

static XMFLOAT3 Add(const XMFLOAT3& l, const XMFLOAT3& r)
{
	return XMFLOAT3(l.x + r.x, l.y + r.y, l.z + r.z);
}

static void Add(XMFLOAT3* a, const XMFLOAT3& b)
{
	a->x += b.x;
	a->y += b.y;
	a->z += b.z;
}

static XMFLOAT3 Multiply(XMFLOAT3& l, XMFLOAT3& r)
{
	return XMFLOAT3(l.x * r.x, l.y * r.y, l.z * r.z);
}
static XMFLOAT3 Multiply(const XMFLOAT3& l, const float b)
{
	return XMFLOAT3(l.x * b, l.y * b, l.z * b);
}


static void Multiply(XMFLOAT3* a, const  float b)
{
	a->x *= b;
	a->y *= b;
	a->z *= b;
}

static XMFLOAT3 Subtract(const XMFLOAT3& l, const XMFLOAT3& r)
{
	return XMFLOAT3(l.x - r.x, l.y - r.y, l.z - r.z);
}

static float SelfDot(const XMFLOAT3& a)
{
	return a.x + a.y + a.z;
}

static XMFLOAT3 Divide(const XMFLOAT3& a, const float b)
{
	return XMFLOAT3(a.x / b, a.y / b, a.z / b);
}

static float Length(const XMFLOAT3& a)
{
	return sqrt(pow(a.x, 2) + pow(a.y, 2) + pow(a.z, 2));
}

static XMFLOAT3 Normalise(const XMFLOAT3& a)
{
	return Divide(a, Length(a));
}