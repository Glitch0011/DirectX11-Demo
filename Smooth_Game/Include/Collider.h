#include <Component.h>
#include <CachedVariable.h>
#include <PositionData.h>

namespace SmoothGame
{
	class SphereCollider : public Component
	{
		float radius;
		CachedVariable<PositionalData> positionalData;

	public:
		SphereCollider(float radius);
	};
}