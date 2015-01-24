#include <Renderer.h>

using namespace SmoothGame;
using namespace SmoothGraphics;

#include <GameObject.h>
#include <PickingData.h>

Renderer::Renderer()
{
	this->functions[L"Init"] = [=](Params params)
	{
		return this->Init();
	};
}

HRESULT Renderer::Init()
{
	this->graphics = this->GameObject->SendAndRecieve<GraphicsEngine*>(L"getGraphicsEngine");

	if (this->graphics->bufferController->Get(L"BasicConstantBuffer") == nullptr)
	{
		this->graphics->bufferController->CreateConstantBuffer<BasicConstantBuffer>(L"BasicConstantBuffer");
		this->graphics->bufferController->CreateDynamicConstantBuffer<QuickBasicConstantBuffer>(L"QuickBasicConstantBuffer");
		this->graphics->bufferController->CreateDynamicConstantBuffer<AmbientConstantBuffer>(L"AmbientConstantBuffer");
		this->graphics->bufferController->CreateDynamicConstantBuffer<TextureAvaliabilityConstantBuffer>(L"TextureAvaliableConstantBuffer");
		this->graphics->bufferController->CreateDynamicConstantBuffer<PickingData>(L"highlightingData");
	}
	return S_OK;
}

Renderer::~Renderer()
{

}