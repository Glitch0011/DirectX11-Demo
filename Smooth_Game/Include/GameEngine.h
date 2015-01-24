#include <GameObject.h>
#include <GraphicsEngine.h>
#include <vector>
#include <VectorFunctions.h>
#include <functional>
#include <Renderer.h>
#include <map>
#include <string>
#include <InputStorage.h>
#include <chrono>
#include <MouseButtons.h>
#include <Frustum.h>

namespace SmoothGame
{
	class GameEngine
	{
		GameObject* Camera = nullptr;

		std::chrono::system_clock::time_point last;
		std::chrono::system_clock::time_point timeStart;
		
		std::vector<GameObject*> objects;

		XMFLOAT4X4 projectionMatrix;
		HWND hWnd;

		UINT frameRate = 0;
		double nextFrameRate = 0;

		UINT IDcount = 0;

	public:
		UINT lastFrameRate = 0;

		InputStorage inputStorage;

		SmoothGraphics::GraphicsEngine* graphics = nullptr;

		std::map<const std::wstring, Renderer*> renderers;

		Frustum frustum;

		std::function<HRESULT()> onCreateScene = nullptr;
		std::function<HRESULT()> onRender = nullptr;
		std::function<HRESULT(const double&)> onUpdate = nullptr;
		std::function<HRESULT()> onExit = nullptr;

		GameEngine();
		~GameEngine();

		HRESULT Init(HWND g_hWnd);

		void calculatePerspectiveMatrix();

		void UpdateConstantBuffers();

		GameObject* AddObject(MessageName);
		GameObject* AddObject(MessageName, std::vector<Component*>);

		GameObject* SetCamera(GameObject* gameObject);

		void Draw();

		void Update();

		bool MouseEvent(MSG* msg);

		bool onKeyDown(WPARAM msg);

		bool onKeyUp(WPARAM msg);

		bool onMouseDown(MouseButtons button, WPARAM msg);

		bool onMouseUp(MouseButtons button, WPARAM msg);

		GameObject* GetCamera();

		GameObject* operator[] (MessageName name)
		{
			for (unsigned int i = 0; i < this->objects.size(); i++)
			{
				auto obj = this->objects[i];
				if (obj->name == name)
					return obj;
			}
			return nullptr;
		}

		GameObject* Get(MessageName name)
		{
			return this->operator[](name);
		}

		GameObject* GetByID(ID_TYPE id)
		{
			for (auto obj : this->objects)
				if (obj->ID == id)
					return obj;
			
			return nullptr;
		}

		HRESULT DeadendCallback(MessageName functionName, Params params)
		{
			std::map<std::wstring, std::function<HRESULT(Params param)>> functions
			{
				{
					L"getGraphicsEngine",
					[=](Params param)
					{
						((SendAndRecieveWrapper*)param[0])->result = this->graphics;
						return S_OK;
					}
				},
				{
					L"getModel",
					[=](Params param)
					{
						auto v = (SendAndRecieveWrapper*)param[0];
						v->result = this->graphics->modelController->Get(*(std::wstring*)v->params[0]);
						return S_OK;
					}
				},
				{
					L"getInputStorage",
					[=](Params param)
					{
						((SendAndRecieveWrapper*)param[0])->result = &this->inputStorage;
						return S_OK;
					}
				},
				{
					L"getFrustum",
					[=](Params param)
					{
						((SendAndRecieveWrapper*)param[0])->result = &this->frustum;
						return S_OK;
					}
				},
				{
					L"getNewID",
					[=](Params param)
					{
						((SendAndRecieveWrapper*)param[0])->result = (void*)this->IDcount++;
						return S_OK;
					}
				}
			};

			if (functions.count(functionName))
				return functions[functionName](params);

			return S_OK;
		}

		HRESULT Send(MessageName name, Params params = {})
		{
			HRESULT hr = S_OK;
			for (auto itr = this->objects.begin(); itr != this->objects.end(); ++itr)
			{
				hr = (*itr)->Send(name, params);
				if (hr != E_NOTIMPL && FAILED(hr))
					return hr;
			}
			return hr;
		}

		std::vector<GameObject*>::iterator begin()
		{
			return this->objects.begin();
		}
		
		std::vector<GameObject*>::iterator end()
		{
			return this->objects.end();
		}
	};
}