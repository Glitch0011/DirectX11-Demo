#pragma once

#include <vector>
#include <string>
#include <winerror.h>
#include <concurrent_vector.h>
#include <future>
#include <functional>
#include <stdarg.h>
#include <utility>
#include <map>
#include <SmoothGameTypeDefs.h>

namespace SmoothGame
{
	class Component;

	class SendAndRecieveWrapper
	{
	public:
		SendAndRecieveWrapper(Params params)
		{
			this->params = params;
			this->result = nullptr;
		}

		std::vector<void*> params;
		void* result;
	};

	class GameObject
	{
	public:
		std::wstring name;
		GameObject* Parent;
		ID_TYPE ID;
		bool destroy = false;

		std::vector<Component*> Components;

		std::map<std::wstring, std::wstring> variables;

		concurrency::concurrent_vector<ToSendObject> toSendMessages;

		std::function<HRESULT(MessageName name, Params)> deadEndCallback;

		GameObject(MessageName name)
		{
			this->name = name;
		}

		HRESULT Init()
		{
			this->ID = (ID_TYPE)this->SendAndRecieve<void*>(L"getNewID");

			return S_OK;
		}

		HRESULT SendProcess(MessageName functionName, Params parameters, std::function<void(HRESULT, Params)> callback = nullptr);

		HRESULT Send(MessageName functionName);

		HRESULT Send(MessageName functionName, Params parameters);

		HRESULT DelayedSend(MessageName functionName, Params parameters)
		{
			this->toSendMessages.push_back(ToSendObject(functionName, parameters));

			return S_OK;
		}

		HRESULT SendAsync(MessageName functionName, Params param, std::function<void(HRESULT, Params)> callback);

		Component* AddComponent(Component* comp);

		std::wstring GetVariable(std::wstring name)
		{
			if (this->variables.find(name) != this->variables.end())
				return this->variables[name];
			else
				return L"";
		}

		void SetVariable(std::wstring name, std::wstring value)
		{
			this->variables[name] = value;
		}

		template<typename T> T* GetComponent()
		{
			auto result = this->GetComponents<T>();
			return result.size() > 0 ? result[0] : nullptr;
		}

		template<typename T> std::vector<T*> GetComponents()
		{
			std::vector<T*> results;

			for (UINT i = 0; i < this->Components.size(); i++)
			{
				Component* component = this->Components[i];

				if (T* p = dynamic_cast<T*>(component))
				{
					results.push_back(p);
				}
			}

			return results;
		}

		template<typename T> T SendAndRecieve(MessageName functionName)
		{
			return this->SendAndRecieve<T>(functionName, { });
		}

		template<typename T> T SendAndRecieve(MessageName functionName, Params _params)
		{
			SendAndRecieveWrapper wrapper(_params);

			auto res = this->Send(functionName, { &wrapper });

			if (SUCCEEDED(res))
			{
				T result = (T)wrapper.result;
				return result;
			}
			else
				return nullptr;
		}

		template<typename T> void SendAndRecieveAsync(MessageName functionName, std::function<void(T)> callback)
		{
			this->SendAndRecieveAsync(functionName, {}, callback);
		}

		template<typename T> void SendAndRecieveAsync(MessageName functionName, Params _params, std::function<void(T)> callback)
		{
			SendAndRecieveWrapper* wrapper = new SendAndRecieveWrapper(_params);

			this->SendAsync(functionName, { wrapper }, [=](HRESULT res, Params params)
			{
				auto _wrapper = (SendAndRecieveWrapper*)params[0];
				if (SUCCEEDED(res))
				{
					callback((T)_wrapper->result);
				}
				else
				{
					callback(nullptr);
				}

				delete _wrapper;
			});
		}
	};
}