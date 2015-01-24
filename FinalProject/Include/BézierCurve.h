#include "DirectXMath.h"
#include <vector>
#include <iostream>
#include <cstdarg>

using namespace DirectX;

class BézierCurve
{
	std::vector<XMFLOAT3> controlPoints;

public:
	BézierCurve(int numberOfArgs, const XMFLOAT3 list...)
	{
		va_list args;
		va_start(args, list);

		controlPoints.push_back(list);

		for (int i = 0; i < numberOfArgs - 1; i++)
			controlPoints.push_back(va_arg(args, XMFLOAT3));

		va_end(args);
	}

	float interpolate(float a, float b, float t)
	{
		return a + ((b - a) * t);
	}

	XMFLOAT3 interpolate(XMFLOAT3 a, XMFLOAT3 b, float t)
	{
		return XMFLOAT3(
			this->interpolate(a.x, b.x, t),
			this->interpolate(a.y, b.y, t),
			this->interpolate(a.z, b.z, t));
	}

	std::vector<XMFLOAT3> recursePoints(std::vector<XMFLOAT3> points, float t)
	{
		std::vector<XMFLOAT3> ret;

		for (auto i = 0; i < points.size() - 1; i++)
			ret.push_back(this->interpolate(points[i], points[i + 1], t));

		return ret.size() != 1 ? recursePoints(ret, t) : ret;
	}

	XMFLOAT3 GetAt(float t)
	{
		return this->recursePoints(this->controlPoints, t)[0];
	}
};