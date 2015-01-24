#include <FPSControllerComponent.h>

#include <GameObject.h>
#include <vector>
#include <PositionData.h>
#include <VectorFunctions.h>

using namespace SmoothGame;
using namespace std;

FPSControllerComponent::FPSControllerComponent()
{
	this->functions[L"forward"] = [=](Params param)
	{
		return S_OK;
	};

	this->functions[L"Update"] = [=](Params param)
	{
		auto timePassedInSeconds = *((double*)param[0]);
		auto rotSpeed = (float)(timePassedInSeconds * XM_PIDIV4);

		auto objData = this->GameObject->SendAndRecieve<PositionalData*>(L"getPositionData");

		if (objData != nullptr)
		{
			if (this->input.up)
			{
				objData->Rotation()->x += rotSpeed;
			}
			if (this->input.left)
			{
				objData->Rotation()->y += rotSpeed;
			}
			if (this->input.right)
			{
				objData->Rotation()->y -= rotSpeed;
			}
			if (this->input.down)
			{
				objData->Rotation()->x -= rotSpeed;
			}
		}

		float moveSpeed = (float)(timePassedInSeconds * 100);
		auto speed = XMFLOAT3(moveSpeed, moveSpeed, moveSpeed);

		if (this->input.forward)
		{
			*objData->Position() = Subtract(*objData->Position(), Multiply(objData->ForwardVector(), speed));
		}
		if (this->input.backwards)
		{
			*objData->Position() = Add(*objData->Position(), Multiply(objData->ForwardVector(), speed));
		}

		return S_OK;
	};
}