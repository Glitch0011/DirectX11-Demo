#include <direct.h>
#include <GraphicsEngine.h>
#include <Component.h>

using namespace SmoothGraphics;

namespace SmoothGame
{
	struct TimeConstantBuffer
	{
		double time;
		double timePassed;
	};

	class ComputeComponent : public Component
	{
	private:
		GraphicsEngine* graphics;
		std::wstring shaderName;
		XMINT3 batchSize;

	public:
		ComputeComponent(GraphicsEngine* graphics, std::wstring shaderName, XMINT3 batchSize);
	};
}