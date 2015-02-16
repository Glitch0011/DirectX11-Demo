#include <TopDownControllerInput.h>

#include <GameObject.h>
#include <vector>
#include <VectorFunctions.h>
#include <XInputComponent.h>

using namespace SmoothGame;
using namespace std;

TopDownControllerComponent::TopDownControllerComponent() : Component()
{
	objData = CachedVariable<PositionalData>(TO_FUNCTION(this->GameObject->SendAndRecieve<PositionalData*>(L"getPositionData")));

	this->functions[L"setMove"] = [=](Params param)
	{
		moveAxis = *(XMFLOAT2*)param[0];

		return S_OK;
	};

	this->functions[L"Update"] = [=](Params param)
	{
		auto timePassedInSeconds = *((double*)param[0]);

		auto speed = timePassedInSeconds * 5;

		if (objData != nullptr)
		{
			XMFLOAT3 force = XMFLOAT3(0, 0, 0);

			if (this->input.up)
			{
				force.x += speed;
			}
			if (this->input.left)
			{
				force.y += speed;
			}
			if (this->input.right)
			{
				force.y -= speed;
			}
			if (this->input.down)
			{
				force.x -= speed;
			}

			auto thumbSpeed = 7.5f;

			force.x += (moveAxis.x / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) * timePassedInSeconds * thumbSpeed;
			force.y += (moveAxis.y / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) * timePassedInSeconds * thumbSpeed;

			this->accelerometer->AddVelocity(force);
		}

		this->accelerometer->Update(timePassedInSeconds);

		return S_OK;
	};
}

HRESULT TopDownControllerComponent::Init()
{
	auto res = Component::Init();

	if (this->objData != nullptr)
		accelerometer = new Accelerometer(this->objData->Position());

	return res;
}