#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <memory>
#include <resource.h>

#include <GraphicsEngine.h>
#include <ConstantBuffer.h>

#include <GameObject.h>
#include <time.h>
#include <GameEngine.h>
#include <BillboardRendererComponent.h>
#include <PositionComponent.h>
#include <FPSControllerComponent.h>
#include <ComputeComponent.h>
#include <StructuredBufferComponent.h>
#include <KeyListenerComponent.h>
#include <HttpComponent.h>
#include <MouseListenerComponent.h>
#include <XInputComponent.h>
#include <UpdateComponent.h>
#include <ModelRenderer.h>
#include <Collider.h>
#include <TopDownControllerInput.h>

#include <FileSystem.h>
#include <SetingsFile.h>
#include <list>
#include <random>
#include <LightComponent.h>

#include <ColourHelper.h>

#include <OnClickHighlight.h>
#include <PickingData.h>

#include <BézierCurve.h>
#include <PotentialEngine.h>

using namespace std;
using namespace DirectX;

using namespace SmoothGraphics;
using namespace SmoothGame;
using namespace SmoothHardware;

HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;

class Game
{
public:
	GameEngine* engine = nullptr;
	double lastAvgFramerate = 0;

	Game()
	{
		engine = new GameEngine();
	}

	~Game()
	{
		delete engine;
	}


	HRESULT Init(HINSTANCE hInstance, int nCmdShow)
	{
		if (FAILED(InitWindow(hInstance, nCmdShow)))
			return 0;

		auto frameRate = new std::list<double>();

		engine->onCreateScene = [this]()
		{
			bool renderFloor = true;

			//Creates Camera giving it a position and a FPSController (along with XInput for demo only)
			auto camera = this->engine->SetCamera(this->engine->AddObject(L"camera",
			{
				new PositionComponent(
				[=](PositionalData* data)
				{
					*data->Position() = XMFLOAT3(0, 0, 0.5);
				}),
				new XInputComponent(
					1,
					[=](Params params, XInputComponent* cThis)
					{
						if (cThis->GetState().Gamepad.bLeftTrigger)
							cThis->Vibrate(60000, 60000);
						else
							cThis->Vibrate(0, 0);
						return S_OK;
					}),
			}));

			camera->AddComponent(new KeyListenerComponent(
			{
				KeyListenEvent(' ', [&]
				{
					this->engine->Send(L"makeImage");
				}),
				KeyListenEvent('B', [&]
				{
					this->engine->Send(L"fluxColours");
				}),
			}));

			//Setup Game Engine
			int subSize = 2;

			int size = subSize * subSize * subSize * (16 * 16 * 4);
			this->engine->AddObject(L"particles", {
				new PositionComponent([](PositionalData* data)
				{

				}),
				new BillboardRendererComponent(size),
			});

			//Setup Player
			auto player = this->engine->AddObject(L"Player", {
				new PositionComponent([&](PositionalData* data){}),
				new TopDownControllerComponent(),
			});
			auto topDownController = player->GetComponent<TopDownControllerComponent>();
			player->AddComponent(
				new KeyListenerComponent({
				KeyListenEvent(38, [=]{topDownController->input.left = true; }, [=]{topDownController->input.left = false; }),
				KeyListenEvent(40, [=]{topDownController->input.right = true; }, [=]{topDownController->input.right = false; }),
				KeyListenEvent(39, [=]{topDownController->input.up = true; }, [=]{topDownController->input.up = false; }),
				KeyListenEvent(37, [=]{topDownController->input.down = true; }, [=]{topDownController->input.down = false; }),
			}));

			auto p = std::vector < PositionComponent* > { player->GetComponent<PositionComponent>() };

			//Setup Game
			this->engine->AddObject(L"PotentialEngine", {
				new ComputeComponent(XMINT3(subSize, subSize, subSize)),
				new PotentialEngine(p),
			});

			//Load the previous settings
			DirectoryIteractor end_itr;
			for (DirectoryIteractor itr(Path(L".\\..\\Assets")); itr != end_itr; ++itr)
			{
				if (itr->has_extension())
				{
					if (itr->extension() == L"cfg")
					{
						SettingsFile file = SettingsFile(*itr);
						auto settings = file.getSettings();

						if (settings->find(L"AvgFramerate") != settings->end())
						{
							auto data = settings->at(L"AvgFramerate");
							lastAvgFramerate = std::wcstod(data.c_str(), nullptr);
						}
					}
				}
			}

			return S_OK;
		};

		engine->onRender = []()
		{
			return S_OK;
		};

		engine->onUpdate = [=](const double& timePassed)
		{
			frameRate->push_back(timePassed);
			if (frameRate->size() > 100)
				frameRate->pop_front();

			double sum = 0.0;
			for (auto itr = frameRate->begin(); itr != frameRate->end(); ++itr)
				sum += *itr;
			double avg = sum / frameRate->size();

			std::wstring text = L"";
			std::wstring baseText = L"Loaded Models: " + std::to_wstring(this->engine->graphics->modelController->Models().size()) + L" ";
			if (this->lastAvgFramerate != 0)
				text = 
					baseText +
					L"SmoothGraphics - FPS: " +
					std::to_wstring(this->engine->lastFrameRate) +
					L" PS:" + std::to_wstring(avg) +
					L" vs " + std::to_wstring(this->lastAvgFramerate);
			else
				text = 
					baseText +
					L"SmoothGraphics: " + 
					std::to_wstring(avg);


			SetWindowText(g_hWnd, text.c_str());

			return S_OK;
		};

		engine->onExit = [=]()
		{
			double sum = 0.0;
			for (auto itr = frameRate->begin(); itr != frameRate->end(); ++itr)
				sum += *itr;
			double avg = sum / frameRate->size();

			SettingsFile file = SettingsFile(Path(L"./../Assets/settings.cfg"));
			file.SetSetting(L"AvgFramerate", to_wstring(avg));
			file.Save();

			delete frameRate;

			return S_OK;
		};
		
		engine->Init(g_hWnd);

		//std::this_thread::sleep_for(std::chrono::seconds(1));

		return S_OK;
	}

	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
	{
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = Game::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(Game*);
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_GAME_ICON);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = L"TutorialWindowClass";
		wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_GAME_ICON);

		if (!RegisterClassEx(&wcex))
			return E_FAIL;

		g_hInst = hInstance;
		RECT rc = { 0, 0, 1280, 720 };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		g_hWnd = CreateWindow(
			L"TutorialWindowClass",
			L"Direct3D 11 Tutorial 404: Lets make noise",
			WS_OVERLAPPEDWINDOW,
			0, 0, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
			NULL);

		//Store 'this' in the window's data
		SetWindowLongPtr(g_hWnd, 0, reinterpret_cast<LONG_PTR>(this));

		if (!g_hWnd)
			return E_FAIL;

		ShowWindow(g_hWnd, nCmdShow);

		return S_OK;
	}

	void onResize()
	{
		this->engine->calculatePerspectiveMatrix();
	}

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto p = GetWindowLongPtrW(hWnd, 0);
		auto pWnd = reinterpret_cast<Game*>(p);

		switch (message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_SIZE:
			pWnd->onResize();
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0L;
	}

	HRESULT Run()
	{
		MSG msg = { 0 };

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				bool fHandled = false;
				if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)
				{
					if (msg.message == WM_LBUTTONDOWN)
						fHandled = engine->onMouseDown(MouseButtons::Lmb, msg.lParam);
					else if (msg.message == WM_LBUTTONUP)
						fHandled = engine->onMouseUp(MouseButtons::Lmb, msg.lParam);
					else if (msg.message == WM_RBUTTONDOWN)
						fHandled = engine->onMouseDown(MouseButtons::Rmb, msg.lParam);
					else if (msg.message == WM_RBUTTONUP)
						fHandled = engine->onMouseUp(MouseButtons::Rmb, msg.lParam);
				}
				else if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
				{
					if (msg.message == WM_KEYDOWN)
						fHandled = engine->onKeyDown(msg.wParam);
					else if (msg.message == WM_KEYUP)
						fHandled = engine->onKeyUp(msg.wParam);
				}
				else if (WM_QUIT == msg.message)
					break;

				if (!fHandled)
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			else
			{
				if (engine)
				{
					engine->Update();
					engine->Draw();
				}
			}
		}

		return (HRESULT)msg.lParam;
	}
};