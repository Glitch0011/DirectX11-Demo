#pragma once

#define WIN32_LEAN_AND_MEAN

#define XINPUT_ENABLED

#include <Windows.h>

#ifdef XINPUT_ENABLED
	#include <Xinput.h>
	#pragma comment(lib, "XInput.lib")
#endif

#include <UpdateComponent.h>
#include <functional>

namespace SmoothGame
{
	class XInputComponent : public Component
	{
	public:

#ifdef XINPUT_ENABLED
		XINPUT_STATE controllerState;

		int controllerNum;
		std::function<HRESULT(std::vector<void*>, XInputComponent* cThis)> target;

		XInputComponent(int playerNumber, std::function<HRESULT(std::vector<void*>, XInputComponent* cThis)> onUpdate)
		{
			controllerNum = playerNumber - 1;
			this->target = onUpdate;

			this->functions[L"Update"] = [=](std::vector<void*> params)
			{
				return this->target(params, this);
			};
		}

		XINPUT_STATE GetState()
		{
			ZeroMemory(&controllerState, sizeof(XINPUT_STATE));

			XInputGetState(controllerNum, &controllerState);

			return controllerState;
		}

		bool IsConnected()
		{
			ZeroMemory(&controllerState, sizeof(XINPUT_STATE));

			DWORD Result = XInputGetState(controllerNum, &controllerState);

			if (Result == ERROR_SUCCESS)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		void Vibrate(int leftVal, int rightVal)
		{
			// Create a Vibraton State
			XINPUT_VIBRATION Vibration;

			// Zeroise the Vibration
			ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));

			// Set the Vibration Values
			Vibration.wLeftMotorSpeed = leftVal;
			Vibration.wRightMotorSpeed = rightVal;

			// Vibrate the controller
			XInputSetState(controllerNum, &Vibration);
		}
#endif
	};
}
