#include <Game.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	auto game = make_shared<Game>();

	game->Init(hInstance, nCmdShow);

	auto result = game->Run();

	return (int)result;
}