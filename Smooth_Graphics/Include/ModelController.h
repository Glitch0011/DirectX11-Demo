#pragma once

#include <d3d11.h>
#include <NamedResource.h>
#include <vector>
#include <Model.h>
#include <concurrent_vector.h>

using namespace std;
using namespace DirectX;

namespace SmoothGraphics
{
	struct ModelVertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TexCord;
	};

	class ModelController
	{
		concurrency::concurrent_vector<Model*> models;

	public:
		ModelController();

		~ModelController()
		{
			for each (auto model in models)
			{
				delete model;
			}
		}

		HRESULT Init();

		Model* Get(std::wstring name)
		{
			for (auto itr = models.begin(); itr != models.end(); ++itr)
			{
				if ((*itr)->name == name)
				{
					return *itr;
				}
			}

			return nullptr;
		}

		concurrency::concurrent_vector<Model*> Models()
		{
			return models;
		}

		void GenerateSphere();

		vector<ModelVertex> calculateVerticies(float radius, float numberOfRings, float numberOfSlicesPerRing);

		vector<UINT> calculateInidies(float numberOfRings, float numberOfSlicesPerRing, int numOfRingVerticies, int numOfVerticies);
	};
}