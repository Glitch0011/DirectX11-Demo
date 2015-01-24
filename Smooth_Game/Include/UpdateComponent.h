#pragma once

#include <Component.h>
#include <functional>
#include <vector>

namespace SmoothGame
{
	class UpdateComponent : public Component
	{
	public:
		std::function<HRESULT(Params)> target;

		UpdateComponent(std::function<HRESULT(Params)> target)
		{
			this->functions[L"Update"] = [=](Params params)
			{
				if (this->target)
					return this->target(params);

				return E_INVALIDARG;
			};

			this->target = target;
		}
	};
}