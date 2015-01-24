#pragma once

#include <Component.h>
#include <LightData.h>

namespace SmoothGame
{
	enum LightType
	{
		LightType_PointLight = 0, LightType_SpotLight, LightType_DirectionLight
	};

	class LightComponent : public Component
	{
		LightData lightData; 
		LightData tmp;

		GraphicsEngine* graphics;
		XMFLOAT3* posData;
		XMFLOAT3* rotData;

	public:
		LightComponent(XMFLOAT3 attr, XMFLOAT3 diffuse, float range, XMFLOAT3 colour, LightType type = LightType_PointLight, float angle = 360.0f, XMFLOAT3 Direction = XMFLOAT3(0, 0, 0))
			: Component()
		{
			tmp.Attr = attr;
			tmp.Diffuse = diffuse;
			tmp.Range = range;
			tmp.Colour = colour;
			tmp.Type = type;
			tmp.Direction = Direction;
			tmp.Angle = angle;

			this->functions[L"Update"] = [=](Params params)
			{
				if (posData && 
					(posData->x != this->lightData.Pos.x || posData->y != this->lightData.Pos.y || posData->z != this->lightData.Pos.z) ||
					(rotData->x != this->tmp.Direction.x || rotData->y != this->tmp.Direction.y || rotData->z != this->tmp.Direction.z))
				{
					this->lightData.Pos = *posData;

					//Since the direction is translated from Rotation we need to store the untranslated version to allow quick check
					this->tmp.Direction = *rotData;

					//Transform a direction into the rotated direction
					{
						auto m =
							XMMatrixRotationX(rotData->x) *
							XMMatrixRotationY(rotData->y) *
							XMMatrixRotationZ(rotData->z);

						XMFLOAT3 target = XMFLOAT3(0, 0, 1);

						XMStoreFloat3(&this->lightData.Direction, XMVector3Transform(XMLoadFloat3(&target), m));
					}

					this->graphics->lightController->UpdateLight();
				}

				return S_OK;
			};

			this->functions[L"Init"] = [=](Params params)
			{
				graphics = this->GameObject->SendAndRecieve<GraphicsEngine*>(L"getGraphicsEngine"); 

				if (SUCCEEDED(graphics->lightController->CreateLight(&lightData)))
				{
					lightData.Attr = tmp.Attr;
					lightData.Colour = tmp.Colour;
					lightData.Diffuse = tmp.Diffuse;
					lightData.Range = tmp.Range;
					lightData.Type = tmp.Type;
					lightData.Angle = tmp.Angle * 0.0174532925;

					graphics->lightController->UpdateLight();
				}

				auto p = &this->GameObject->GetComponent<PositionComponent>()->positionalData;

				this->posData = p->Position();
				this->rotData = p->Rotation();

				return S_OK;
			};
		}
	};

	class SpotLightComponent : public LightComponent
	{
	public:
		SpotLightComponent(XMFLOAT3 attr, XMFLOAT3 diffuse, float range, XMFLOAT3 colour, float angle)
			: LightComponent(attr, diffuse, range, colour, LightType_SpotLight, angle)
		{

		}
	};

	class DirectionLightComponent : public LightComponent
	{
	public:
		DirectionLightComponent(XMFLOAT3 attr, XMFLOAT3 diffuse, float range, XMFLOAT3 colour)
			: LightComponent(attr, diffuse, range, colour, LightType_DirectionLight)
		{

		}
	};
}