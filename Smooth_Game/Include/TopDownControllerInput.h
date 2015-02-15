#pragma once

#include <Component.h>
#include <InputStorage.h>

namespace SmoothGame
{
	struct TopDownControllerInput
	{
		bool left, right, up, down;
	};

	class TopDownControllerComponent : public Component
	{
	public:
		TopDownControllerInput input;

		TopDownControllerComponent();
	};
}