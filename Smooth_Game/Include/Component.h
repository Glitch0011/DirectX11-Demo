#pragma once

#include <map>
#include <string>
#include <functional>
#include <vector>
#include <winerror.h>
#include <SmoothGameTypeDefs.h>

namespace SmoothGame
{
	class GameObject;

	class Component
	{
	protected:
		std::map<std::wstring, std::function<HRESULT(Params)>> functions;
		std::vector<Component*> children;

	public:
		GameObject* GameObject;
		bool destroy = false;

		virtual ~Component()
		{

		}

		virtual HRESULT Init()
		{
			return S_OK;
		}

		HRESULT RecieveMessage(MessageName functionName, Params parameters);

		HRESULT Send(MessageName name, Params param);

		HRESULT DelayedSend(MessageName name, Params param = {});

		HRESULT SendAsync(MessageName name, Params param, std::function<void(HRESULT, Params)>);
	};
}