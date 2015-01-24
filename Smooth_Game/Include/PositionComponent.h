#pragma once

#include <Component.h>
#include <PositionData.h>
#include <DirectXMath.h>
#include <vector>
#include <functional>

using namespace std;
using namespace DirectX;

namespace SmoothGame
{
	class PositionComponent : public Component
	{
	public:
		PositionalData positionalData;

		PositionComponent(std::function<void(PositionalData*)> callback = 0)
		{
			this->functions[L"getPositionData"] = [=](Params param)
			{
				((SendAndRecieveWrapper*)param[0])->result = &this->positionalData;
				return S_OK;
			};

			*this->positionalData.Scale() = XMFLOAT3(1, 1, 1);

			callback(&this->positionalData);
		}
	};
}