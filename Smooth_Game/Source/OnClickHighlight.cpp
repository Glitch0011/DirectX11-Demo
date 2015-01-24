#include <OnClickHighlight.h>

#include <GameObject.h>
#include <ModelRenderer.h>

using namespace SmoothGame;

OnClickHighlight::OnClickHighlight()
{
	this->functions[L"Update"] = [=](Params params)
	{
		return S_OK;
	};
	this->functions[L"onClick"] = [=](Params params)
	{
		auto p = (SmoothGraphics::PickingData*)params[0];

		//this->GameObject->destroy = true;

		auto modelRenderer = this->GameObject->GetComponent<ModelRenderer>();

		modelRenderer->highlightData = *p;
			
		return S_OK;
	};
}