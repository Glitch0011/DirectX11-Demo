#include <direct.h>
#include <GraphicsEngine.h>
#include <Component.h>
#include <Renderer.h>
#include <CachedVariable.h>

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

		CachedVariable<StructuredBuffer> objectBuffer,movingBuffer, playerBuffer;
		CachedVariable<vector<PlayerData>> playerData;
		CachedVariable<ConstantBuffer> timeConstBuffer, elementConstBuffer;

	public:
		XMINT3 batchSize;

		ComputeComponent(XMINT3 batchSize);

		HRESULT Init();
		HRESULT Call(
			std::wstring& shaderName, 
			std::vector<SmoothGraphics::ConstantBuffer*>& constantBuffers,
			std::vector<SmoothGraphics::StructuredBuffer*>& uavBuffers,
			DirectX::XMINT3& batchSize
			);
	};
}