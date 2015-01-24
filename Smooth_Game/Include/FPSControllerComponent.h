#pragma once

#include <Component.h>
#include <InputStorage.h>

namespace SmoothGame
{
	struct FPSControllerInput
	{
		bool forward, backwards, left, right, up, down;
	};

	class FPSControllerComponent : public Component
	{
	public:
		FPSControllerInput input;

		FPSControllerComponent();
	};
}