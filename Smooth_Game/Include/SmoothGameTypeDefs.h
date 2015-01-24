#pragma once

#include <string>

namespace SmoothGame
{
	typedef const std::wstring& MessageName;
	typedef const std::vector<void*>& Params;
	typedef const std::function<HRESULT(Params)>& Callback;
	typedef std::pair<std::wstring, Params> ToSendObject;
	typedef unsigned int ID_TYPE;
}