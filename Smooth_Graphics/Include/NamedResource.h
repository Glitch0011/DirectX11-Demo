#pragma once

#include <string>

namespace SmoothGraphics
{
	class NamedResource
	{
	public:
		std::wstring name;

		NamedResource(std::wstring name)
		{
			this->name = name;
		}
	};
}