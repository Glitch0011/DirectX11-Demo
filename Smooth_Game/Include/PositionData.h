#pragma once

#include <d3d11.h>
#include <directxmath.h>

using namespace DirectX;

namespace SmoothGame
{
	class PositionalData
	{
	private:
		XMFLOAT3 position;
		XMFLOAT3 rotation;
		XMFLOAT3 scale;

	public:
		XMFLOAT3* Position();

		XMFLOAT3* Scale();

		XMFLOAT3* Rotation();

		XMFLOAT3 ForwardVector()
		{
			auto k = XMFLOAT3(0, 0, 1);
			XMFLOAT3 t;
			auto l = *this->Rotation();

			l.x *= -1;
			l.y *= -1;
			l.z *= -1;

			XMStoreFloat3(&t, XMVector3Transform(XMLoadFloat3(&k), XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&l))));
			return t;
		}
	};
}