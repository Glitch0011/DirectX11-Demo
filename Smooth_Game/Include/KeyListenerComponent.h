#pragma once

#include <Component.h>
#include <InputStorage.h>
#include <functional>
#include <string>
#include <InputListenerComponent.h>

namespace SmoothGame
{
	class KeyListenEvent
	{
	public:
		char key;
		std::function<void()> onKeyDown;
		std::function<void()> onKeyUp;

		bool keyUp;

		KeyListenEvent(char key, std::function<void()> function, bool keyUp = false)
		{
			this->key = key;
			this->onKeyDown = function;
			this->keyUp = keyUp;
		}
		KeyListenEvent(char key, std::function<void()> onKeyDown, std::function<void()> onKeyUp)
		{
			this->key = key;
			this->onKeyDown = onKeyDown;
			this->onKeyUp = onKeyUp;
			this->keyUp = true;
		}
	};

	class KeyListenerComponent : public InputListenerComponent
	{
		std::vector<KeyListenEvent> events;

	public:
		KeyListenerComponent(std::initializer_list<KeyListenEvent> events)
		{
			this->events = events;

			this->functions[L"Update"] = [=](Params params)
			{
				if (this->inputStorage)
				{
					for (auto eve : this->events)
					{

						if (eve.keyUp ? this->inputStorage->lastKeys[eve.key] == 1 && this->inputStorage->keys[eve.key] == 0 : this->inputStorage->keys[eve.key])
						{
							if (eve.onKeyUp)
								eve.onKeyUp();
						}
						if (eve.keyUp ? this->inputStorage->lastKeys[eve.key] == 0 && this->inputStorage->keys[eve.key] == 1 : this->inputStorage->keys[eve.key])
						{
							if (eve.onKeyDown)
								eve.onKeyDown();
						}
					}
				}

				return S_OK;
			};

			InputListenerComponent();
		}
	};
}