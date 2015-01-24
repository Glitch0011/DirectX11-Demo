#include <Component.h>
#include <GameObject.h>

using namespace std;
using namespace SmoothGame;

HRESULT Component::Send(MessageName name, Params param)
{
	return this->GameObject->Send(name, param);
}

HRESULT Component::RecieveMessage(MessageName functionName, Params parameters)
{
	if (this->functions.size() > 0)
	{
		for (auto functionDeclaration : this->functions)
		{
			if (functionDeclaration.first == functionName)
			{
				return functionDeclaration.second(parameters);
			}
		}
	}

	return E_NOTIMPL;
}

HRESULT Component::DelayedSend(MessageName funtionName, Params parameters)
{
	return this->GameObject->DelayedSend(funtionName, parameters);
}

HRESULT Component::SendAsync(MessageName name, Params param, function<void(HRESULT, Params)> callback)
{
	return this->GameObject->SendAsync(name, param, callback);
}