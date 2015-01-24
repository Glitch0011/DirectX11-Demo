#include <Component.h>

namespace SmoothGame
{
	class HttpComponent : public Component
	{
		volatile int running;

	public:
		HttpComponent();
	};
}