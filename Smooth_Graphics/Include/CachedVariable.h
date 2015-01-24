#pragma once

#include <functional>

using namespace std;

#define TO_FUNCTION(x) [=](){return x;}

template<class T> class CachedVariable
{
public:
	typedef std::function<T*()> GetFunction;
	T* value;
	GetFunction getFunction;

	CachedVariable(GetFunction getFunction)
	{
		this->value = nullptr;
		this->getFunction = getFunction;
	}

	CachedVariable()
	{
		this->value = nullptr;
		this->getFunction = nullptr;
	}

	T* check()
	{
		if (value)
			return value;
		else if (this->getFunction)
		{
			this->value = this->getFunction();
			return this->value;
		}
		return nullptr;
	}

	void Try(std::function<void(T*)> toTry)
	{
		if (*this)
		{
			toTry();
		}
	}

	operator T*()
	{
		return check();
	}

	T* operator-> ()
	{
		return check();
	}

	void Clear()
	{
		this->value = nullptr;
	}

	T*const* GetPtr()
	{
		if (this->check())
		{
			return &this->value;
		}
	}
};