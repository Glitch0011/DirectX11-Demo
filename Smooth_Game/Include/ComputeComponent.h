#include <direct.h>
#include <GraphicsEngine.h>
#include <Component.h>
#include <Renderer.h>

using namespace SmoothGraphics;

namespace SmoothGame
{
	struct TimeConstantBuffer
	{
		double time;
		double timePassed;
	};

	struct ElementDataBuffer
	{
		DirectX::XMUINT4 batchSize;
	};

	class ComputeComponent : public Renderer
	{
	private:
		std::wstring shaderName;
		XMINT3 batchSize;

	public:
		ComputeComponent(XMINT3 batchSize);
		HRESULT Init();
	};
}