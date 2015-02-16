#include <PotentialEngine.h>

#include <GameObject.h>
#include <vector>
#include <PositionData.h>
#include <VectorFunctions.h>
#include <ComputeComponent.h>

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
		auto timePassed = static_cast<double*>(param[0]);

		for (int i = 0; i < playerPositions.size(); i++)
		{
			this->playerData[i].Pos = *playerPositions[i]->positionalData.Position();
		}

		if (this->pulses.size() > 0)
		{
			auto pulseConstantBuffer = this->graphics->Get<ConstantBuffer>(L"PulseConstantBuffer");

			for (auto& pulse : this->pulses)
			{
				if (pulse.timeLeft > 0)
				{
					PulseData data; ZeroMemory(&data, sizeof(PulseData));
					data.Strength = ((pulse.timeMax - pulse.timeLeft) / pulse.timeMax) * pulse.strength;
					data.Padding1 = pulse.timeMax * pulse.strength;

					pulseConstantBuffer->Update(this->graphics->immediateContext, &data);

					std::wstring shaderName = L"Pulse";
					std::vector<ConstantBuffer*> extraBuffers{ pulseConstantBuffer };

					this->Send(L"Compute", { &shaderName, &extraBuffers });

					pulse.timeLeft -= *timePassed;
				}
			}
		}

		return S_OK;
	}; 
	
	this->functions[L"Pulse"] = [=](Params param)
	{
		this->pulses.push_back(Pulse(1));
		return S_OK;
	};

	this->functions[L"Fire"] = [=](Params param)
	{

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

	this->graphics->bufferController->CreateDynamicConstantBuffer<PulseData>(L"PulseConstantBuffer");

	return res;
}