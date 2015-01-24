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

#include <FileSystem.h>
#include <SetingsFile.h>
#include <list>
#include <random>
#include <LightComponent.h>

#include <ColourHelper.h>

#include <OnClickHighlight.h>
#include <PickingData.h>

#include <BézierCurve.h>

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
			UINT cubeCount = 9;//81; Go on... try it...
			UINT lightCount = 9;// 27;

			bool renderFloor = true;

			//Creates Camera giving it a position and a FPSController (along with XInput for demo only)
			auto camera = this->engine->SetCamera(this->engine->AddObject(L"camera",
			{
				new PositionComponent(
				[=](PositionalData* data)
				{
					*data->Position() = XMFLOAT3(230, -200, 230);
					*data->Rotation() = XMFLOAT3(-0.592020512f, -0.802229762f, 0.000000000f);
				}),
					new FPSControllerComponent(),
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

			camera->AddComponent(new UpdateComponent([=](Params params)
			{
				if (camera->GetVariable(L"FollowBez") == L"true")
				{
					auto _t = camera->GetVariable(L"t");
					_t = _t == L"" ? L"0.0" : _t;

					auto t = wcstod(_t.c_str(), nullptr);

					//Create curve
					BézierCurve curve = BézierCurve(5, 
						XMFLOAT3(230, -200, 230), 
						XMFLOAT3(-230, -200, 230), 
						XMFLOAT3(-230, -200, -230),
						XMFLOAT3(230, -200, -230),
						XMFLOAT3(230, -200, 230));

					//Update pos and t
					*camera->GetComponent<PositionComponent>()->positionalData.Position() = curve.GetAt(std::fmod(t += *((double*)params[0]), 10) / 10);

					camera->SetVariable(L"t", std::to_wstring(t));
				}

				return S_OK;
			}));

			//Maps the WASD keys to raise or lower the input flags in FPSController
			camera->AddComponent(new KeyListenerComponent(
			{
				KeyListenEvent('W', [=](){camera->GetComponent<FPSControllerComponent>()->input.up = true; }, [=]() {camera->GetComponent<FPSControllerComponent>()->input.up = false; }),
				KeyListenEvent('A', [=](){camera->GetComponent<FPSControllerComponent>()->input.left = true; }, [=]() {camera->GetComponent<FPSControllerComponent>()->input.left = false; }),
				KeyListenEvent('S', [=](){camera->GetComponent<FPSControllerComponent>()->input.down = true; }, [=]() {camera->GetComponent<FPSControllerComponent>()->input.down = false; }),
				KeyListenEvent('D', [=](){camera->GetComponent<FPSControllerComponent>()->input.right = true; }, [=]() {camera->GetComponent<FPSControllerComponent>()->input.right = false; }),
				KeyListenEvent('Q', [=](){camera->SetVariable(L"FollowBez", L"true"); }, [=](){camera->SetVariable(L"FollowBez", L"false"); }),
			}));

			//When the camera clicks, either raise the FPS forward input, or perform picking
			camera->AddComponent(new MouseListenerComponent(
			{
				MouseListenEvent(MouseButtons::Lmb, [=](){camera->GetComponent<FPSControllerComponent>()->input.forward = true; }, [=](){camera->GetComponent<FPSControllerComponent>()->input.forward = false; }),
				MouseListenEvent(MouseButtons::Rmb,
				nullptr, //Don't do anything on RightMouseDown
				[=]() //RightMouseUp does picking
				{
					POINT p;
					if (GetCursorPos(&p) && ScreenToClient(g_hWnd, &p))
					{
						auto res = this->engine->graphics->TestPixel(XMINT2(p.x, p.y)); //Returns vertex data at that point
						this->engine->GetByID(res.id)->Send(L"onClick", { &res }); //Fires to any component that'll recieve it
					}
				}),
			}));

			auto randomEngine = new mt19937_64(); randomEngine->seed(clock());
			auto floatDistribution = new uniform_real_distribution<float>(0.0f, 1.0);

			//Add Objects
			for (auto i = 0; i < cubeCount; i++)
			{
				GameObject* cube = this->engine->AddObject(L"cube",
				{
					new PositionComponent([=](PositionalData* data)
					{
						*data->Scale() = XMFLOAT3(50, 50, 50);
						*data->Position() = XMFLOAT3(
							((i % (int)sqrt(cubeCount)) * 200),
							0,
							((i / (int)sqrt(cubeCount)) * 200));
						*data->Rotation() = XMFLOAT3(
							(*floatDistribution)(*randomEngine) * XM_2PI,
							(*floatDistribution)(*randomEngine) * XM_2PI,
							(*floatDistribution)(*randomEngine) * XM_2PI);
					}),
					new SphereCollider(50), //Allows object to respond to culling check messages.
					new OnClickHighlight(), //Allows object to respond to onClick messages.
				});

				//Get a random model
				wstring model = L"";
				auto modelRand = (*floatDistribution)(*randomEngine);
				if (modelRand > 0.00) model = L"cube";
				if (modelRand > 0.25) model = L"pyramid";
				if (modelRand > 0.50) model = L"sphere";
				if (modelRand > 0.85) model = L"teapot";

				auto texture = L"texture";
				if (model == L"sphere")
					texture = L"map";

				//Setup model renderer
				auto modelRenderer = (ModelRenderer*)cube->AddComponent(new ModelRenderer(model, texture));

				modelRenderer->SetEnvironmentName(L"spheremap");
				if (model == L"sphere")
					modelRenderer->SetHeightMap(L"heightmap");

				//Give it a random bump map
				auto bump = (*floatDistribution)(*randomEngine);
				if (bump > 0.0) modelRenderer->SetBumpName(L"brick");
				if (bump > 0.25) modelRenderer->SetBumpName(L"8084");
				if (bump > 0.50) modelRenderer->SetBumpName(L"bumpmap");
				if (bump > 0.75) modelRenderer->SetBumpName(L"bump");

				//Declare it's ambient
				modelRenderer->ambient = XMFLOAT4(0.1, 0.1, 0.1, 1.0);

				//Spin the objects by adding an update component
				cube->AddComponent(new UpdateComponent([=](Params params)
				{
					cube->GetComponent<PositionComponent>()->positionalData.Rotation()->y += (*((double*)params[0]) * (XM_PI * 2)) * 0.05;
					return S_OK;
				}));

				camera->AddComponent(new KeyListenerComponent(
				{
					KeyListenEvent('T', [=]()
					{
						cube->SetVariable(L"heightmap", modelRenderer->heightMapName);
						modelRenderer->SetHeightMap(L"");
					}, [=]()
					{
						modelRenderer->SetHeightMap(cube->GetVariable(L"heightmap"));
					}),
				}));
			}

			if (renderFloor)
			{
				//Add the floor
				GameObject* cube = this->engine->AddObject(L"floor",
				{
					new PositionComponent([=](PositionalData* data)
					{
						*data->Position() = XMFLOAT3(250, -50, 250);
						*data->Scale() = XMFLOAT3(50, 1, 50);
					}),
					new OnClickHighlight(),
				});

				//Add model renderer
				auto _m = (ModelRenderer*)cube->AddComponent(new ModelRenderer(L"plane", L"map"));
				_m->SetBumpName(L"bumpmap");
				_m->SetHeightMap(L"heightmap");

				//Make the floor spin
				cube->AddComponent(new UpdateComponent([=](Params param)
				{
					double d = *((double*)param[0]);
					cube->GetComponent<PositionComponent>()->positionalData.Rotation()->y += (d / 10.0f);
					return S_OK;
				}));

				//Add key listen event on T to remove and return the bump map
				camera->AddComponent(new KeyListenerComponent(
				{
					KeyListenEvent('T', [=]()
					{
						_m->SetHeightMap(L"");
					}, [=]()
					{
						_m->SetHeightMap(L"heightmap");
					}),
				}));
			}

			//Add the skybox
			GameObject* sphere = this->engine->AddObject(L"sphere",
			{
				new PositionComponent([=](PositionalData* data)
				{
					*data->Position() = XMFLOAT3(250, 0, 250);
					*data->Scale() = XMFLOAT3(1000, 1000, 1000);
					*data->Rotation() = XMFLOAT3(XM_PIDIV2, 0, 0);
				}),
			});
			auto m = ((ModelRenderer*)sphere->AddComponent(new ModelRenderer(L"sphere", L"spheremap", 1)));
			m->SetPixelShaderName(L"ModelPixelSphereShader");
			m->SetGeometryShaderName(L"");

			//Add the lights
			for (auto i = 0; i < lightCount; i++)
			{
				auto colour = HSL2RGB((*floatDistribution)(*randomEngine), 1.0, 0.5);

				GameObject* cube = this->engine->AddObject(L"light" + to_wstring(i),
				{
					new PositionComponent([=](PositionalData* data)
					{
						auto a = cubeCount > 0 ? sqrt(cubeCount) : 1;
						*data->Position() =  XMFLOAT3((i % (int)a) * 250, 0, (i / a) * 250);
						*data->Scale() = XMFLOAT3(2, 2, 2);
						*data->Rotation() = XMFLOAT3(
							0,
							(*floatDistribution)(*randomEngine) * XM_2PI,
							0);
					}),

					//new LightComponent(
					new SpotLightComponent(
						XMFLOAT3(0.1, 0.0, 0.0005),
						XMFLOAT3(1, 1, 1),
						5000,
						XMFLOAT3(colour.R, colour.G, colour.B)
						,90
						),
					new SphereCollider(10),
					new OnClickHighlight(),
				});

				auto m = (ModelRenderer*)cube->AddComponent(new ModelRenderer(L"light", L"texture"));
				m->ambient = XMFLOAT4(1, 1, 1, 1);
				m->SetEnvironmentName(L"");
			}

			//Add the spinning light key
			camera->AddComponent(new KeyListenerComponent(
			{
				KeyListenEvent('E', [=]()
				{
					engine->Get(L"light0")->SetVariable(L"rotate", L"true");
				}, [=]()
				{
					engine->Get(L"light0")->SetVariable(L"rotate", L"false");
				}),
					KeyListenEvent('R', [=]()
				{
					for (GameObject* obj : *this->engine)
					{
						auto renderer = obj->GetComponent<ModelRenderer>();
						if (renderer)
						{
							renderer->SetEnvironmentName(L"");
						}
					}
				}, [=]()
				{
					for (GameObject* obj : *this->engine)
					{
						auto renderer = obj->GetComponent<ModelRenderer>();
						if (renderer)
						{
							renderer->SetEnvironmentName(L"spheremap");
						}
					}
				}),
			}));

			//If the light should be rotating, then rotate them
			camera->AddComponent(new UpdateComponent([=](Params param)
			{
				auto l = engine->Get(L"light0");
				if (l->GetVariable(L"rotate") == L"true")
				{
					auto offsetStr = l->GetVariable(L"offset");
					auto offset = offsetStr != L"" ? _wtof(offsetStr.c_str()) : 0.0;

					double d = *((double*)param[0]);

					offset -= d;

					for (auto i = 0U; i < lightCount; i++)
					{
						auto degree = offset + ((i / (float)lightCount) * XM_2PI);
						auto center = XMFLOAT3(200, 0, 200);
						auto r = 100;
						*engine->Get(L"light" + to_wstring(i))->GetComponent<PositionComponent>()->positionalData.Position() =
							XMFLOAT3(center.x + ((float)sin(degree) * r), center.y, center.z + ((float)cos(degree) * r));
					}

					l->SetVariable(L"offset", to_wstring(offset));
				}
				else
				{
					for (auto i = 0U; i < lightCount; i++)
					{
						auto t = *((double*)param[0]);

						//Rotate every 2 seconds.
						auto m = XMMatrixRotationY(t * XM_2PI * 0.5);

						auto p = engine->Get(L"light" + to_wstring(i))->GetComponent<PositionComponent>()->positionalData.Rotation();

						p->y += t;

						if (p->y > XM_2PI)
							p->y -= XM_2PI;
					}
				}
				return S_OK;
			}));

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