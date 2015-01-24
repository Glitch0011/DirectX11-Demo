#include <GameObject.h>
#include <Component.h>
#include <windows.h>

using namespace std;
using namespace SmoothGame;

Component* GameObject::AddComponent(Component* comp)
{
	comp->GameObject = this;
	this->Components.push_back(comp);
	return comp;
}

HRESULT GameObject::Send(MessageName functionName)
{
	return this->Send(functionName, { });
}

HRESULT GameObject::SendProcess(MessageName functionName, Params params, std::function<void(HRESULT, Params)> callback)
{
	if (this->destroy)
		return S_OK;

	bool wasHandledBySomething = false;

	HRESULT res = S_OK;

	for (UINT i = 0; i < this->Components.size(); i++)
	{
		if (this->Components[i]->destroy)
			continue;

		//If any of the recieves failed, callback and return
		auto res = this->Components[i]->RecieveMessage(functionName, params);

		if (res != E_NOTIMPL && FAILED(res))
		{
			if (callback)
				callback(res, params);
			return res;
		}

		//If the message was handled
		if (res != E_NOTIMPL)
			wasHandledBySomething = true;
	}

	if (!wasHandledBySomething && this->deadEndCallback)
	{
		HRESULT res = this->deadEndCallback(functionName, params);
		if (callback)
			callback(res, params);
	}
	else
	{
		if (callback)
			callback(res, params);
		return res;
	}
}

HRESULT GameObject::Send(MessageName functionName, Params params)
{
	if (this)
		return this->SendProcess(functionName, params);
	else
		return E_INVALIDARG;
}

HRESULT GameObject::SendAsync(MessageName functionName, Params params, std::function<void(HRESULT, Params)> callback)
{
	std::shared_future<void> asyncFunction = std::async(std::launch::async, [=]()
	{
		this->SendProcess(functionName, params, callback);
	});

	return S_OK;
}