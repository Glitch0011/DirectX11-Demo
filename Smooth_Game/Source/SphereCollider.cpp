#include <Collider.h>

#include <GameObject.h>
#include <Frustum.h>
#include <PositionComponent.h>

using namespace SmoothGame;

SphereCollider::SphereCollider(float radius) : Component()
{
	this->radius = radius;

	positionalData = CachedVariable<PositionalData>(TO_FUNCTION(&this->GameObject->GetComponent<PositionComponent>()->positionalData));

	this->functions[L"isOutsideFrustum"] = [=](Params param)
	{
		SendAndRecieveWrapper* wrapper = (SendAndRecieveWrapper*)param[0];

		Frustum* frustum = (Frustum*)wrapper->params[0];
		
		auto res = frustum->sphereInFrustum(XMLoadFloat3(this->positionalData->Position()), this->radius);

		wrapper->result = (void*)(res == 0);

		return S_OK;
	};
}