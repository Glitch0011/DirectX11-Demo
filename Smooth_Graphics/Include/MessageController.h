#pragma once

#include <functional>
#include <vector>
#include <map>
#include <winerror.h>
#include <concurrent_vector.h>
#include <utility> //std::pair

namespace SmoothGraphics
{
	/*typedef std::vector<void*> Params;
	typedef std::function<HRESULT(Params)> Callback;
	typedef std::pair<std::wstring, Params> ToSendObject;

	class MessageController
	{
	public:
		std::map<std::wstring, Callback> functions;
		concurrency::concurrent_vector<ToSendObject> toSendMessages;

		void AddEventListener(std::wstring name, Callback callback)
		{
			this->functions[name] = callback;
		}

		HRESULT SendMessageOnMainThread(std::wstring functionName, Params parameters)
		{
			toSendMessages.push_back(ToSendObject(functionName, parameters));
			return S_OK;
		}

		HRESULT Send(std::wstring name, Params params)
		{
			auto result = this->functions.find(name);
			if (result != this->functions.end())
			{
				this->functions[name](params);
			}
		}

		HRESULT PumpMessages()
		{
			for (auto msg = toSendMessages.begin(); msg != toSendMessages.end(); ++msg)
			{
				this->Send(msg->first, msg->second);
			}
			toSendMessages.clear();
		}
	};*/
}