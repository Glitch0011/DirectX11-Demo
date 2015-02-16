#include <PotentialEngine.h>

#include <GameObject.h>
#include <vector>
#include <PositionData.h>
#include <VectorFunctions.h>

using namespace SmoothGame;

PotentialEngine::PotentialEngine(std::vector<PositionComponent*> playerPositions) : Renderer()
{
	this->functions[L"getPlayerData"] = [&](Params param)
	{
		((SendAndRecieveWrapper*)param[0])->result = &this->playerData;

		return S_OK;
	};

	this->functions[L"Update"] = [=](Params param)
	{
		for (int i = 0; i < playerPositions.size(); i++)
		{
			this->playerData[i].Pos = *playerPositions[i]->positionalData.Position();
		}

		return S_OK;
	};
}

HRESULT PotentialEngine::Init()
{
	auto res = Renderer::Init();

	SmoothGraphics::PlayerData playerData;
	ZeroMemory(&playerData, sizeof(SmoothGraphics::PlayerData));
	this->playerData.push_back(playerData);

	this->graphics->CreateStructuredBuffer<PlayerData>(L"PlayerData", this->playerData.size(), [&](UINT index, void* _vertex)
	{
		PlayerData* vertex = (PlayerData*)_vertex;
		vertex->Pos = XMFLOAT3(0.0, 0.0, 0.0);
		return S_OK;
	});

	return res;
}