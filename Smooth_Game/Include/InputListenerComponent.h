#pragma once

#include <Component.h>
#include <InputStorage.h>

#include <GameObject.h>

namespace SmoothGame
{
	class InputListenerComponent
		: public Component
	{ 
	protected:
		InputStorage* inputStorage;

	public:
		InputListenerComponent()
		{
			this->inputStorage = nullptr;

			this->functions[L"Init"] = [=](Params param)
			{
				this->GameObject->SendAndRecieveAsync<InputStorage*>(L"getInputStorage", [=](InputStorage* storage)
				{
					this->inputStorage = storage;
				});

				return S_OK;
			};
		}
	};
}