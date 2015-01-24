#pragma once

#include <Component.h>
#include <InputStorage.h>
#include <functional>
#include <string>
#include <MouseButtons.h>
#include <InputListenerComponent.h>

namespace SmoothGame
{
	class MouseListenEvent
	{
	public:
		MouseButtons mouseButton;
		std::function<void()> onKeyDown;
		std::function<void()> onKeyUp;

		MouseListenEvent(MouseButtons mouseButton, std::function<void()> onKeyDown, std::function<void()> onKeyUp = 0)
		{
			this->mouseButton = mouseButton;
			this->onKeyDown = onKeyDown;
			this->onKeyUp = onKeyUp;
		}
	};

	class MouseListenerComponent : public InputListenerComponent
	{
		std::vector<MouseListenEvent> events;

	public:
		MouseListenerComponent(std::initializer_list<MouseListenEvent> events)
		{
			this->events = events;

			this->functions[L"Update"] = [=](Params params)
			{
				if (this->inputStorage)
				{
					for (auto eve : this->events)
					{
						if (this->inputStorage->lastMouse[eve.mouseButton] == 1 && this->inputStorage->mouse[eve.mouseButton] == 0)
						{
							if (eve.onKeyUp)
								eve.onKeyUp();
						}
						if (this->inputStorage->lastMouse[eve.mouseButton] == 0 && this->inputStorage->mouse[eve.mouseButton] == 1)
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