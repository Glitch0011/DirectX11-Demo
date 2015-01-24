#pragma once

#define Key(x) x
#define CharToKey(x) x + 65

namespace SmoothGame
{
	struct InputStorage
	{
		int keys[253];
		int mouse[3];

		int lastKeys[253];
		int lastMouse[3];
	};
}