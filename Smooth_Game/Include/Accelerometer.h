#pragma once

#include <DirectXMath.h>
#include <winerror.h>
#include <VectorFunctions.h>

namespace SmoothGame
{
	class Accelerometer
	{
	public:
		DirectX::XMFLOAT3* Pos;
		DirectX::XMFLOAT3 Accel;
		DirectX::XMFLOAT3 Vel;

		Accelerometer(DirectX::XMFLOAT3* Data)
		{
			this->Pos = Data;
		}

		void AddVelocity(const DirectX::XMFLOAT3& force)
		{
			Add(&this->Accel, force);
		}

		HRESULT Update(const double time)
		{
			Add(&this->Vel, this->Accel);

			this->Accel = DirectX::XMFLOAT3(0, 0, 0);

			Multiply(&this->Vel, 0.97f);

			auto l = Length(this->Vel);
			if (l > 10)
			{
				this->Vel = Multiply(Normalise(this->Vel), 5);
			}

			Add(this->Pos, this->Vel);

			return S_OK;
		}
	};
}