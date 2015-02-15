#include <TopDownControllerInput.h>

#include <GameObject.h>
#include <vector>
#include <PositionData.h>
#include <VectorFunctions.h>

using namespace SmoothGame;
using namespace std;

TopDownControllerComponent::TopDownControllerComponent()
{
	this->functions[L"Update"] = [=](Params param)
	{
		auto timePassedInSeconds = *((double*)param[0]);

		auto objData = this->GameObject->SendAndRecieve<PositionalData*>(L"getPositionData");

		auto speed = timePassedInSeconds * 100;

		if (objData != nullptr)
		{
			if (this->input.up)
			{
				objData->Position()->x += speed;
			}
			if (this->input.left)
			{
				objData->Position()->y += speed;
			}
			if (this->input.right)
			{
				objData->Position()->y -= speed;
			}
			if (this->input.down)
			{
				objData->Position()->x -= speed;
			}
		}

		return S_OK;
	};
}