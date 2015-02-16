#pragma once

#include <Component.h>
#include <InputStorage.h>
#include <Accelerometer.h>
#include <CachedVariable.h>
#include <PositionData.h>

namespace SmoothGame
{
	struct TopDownControllerInput
	{
		bool left, right, up, down;
	};

	class TopDownControllerComponent : public Component
	{
	public:
		Accelerometer* accelerometer;
		CachedVariable<PositionalData> objData;

		TopDownControllerInput input;

		TopDownControllerComponent();

		virtual HRESULT Init();
	};
}