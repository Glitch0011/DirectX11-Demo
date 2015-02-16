#include <vector>
#include <Renderer.h>

#include <SimpleVertex.h>>
#include <PositionComponent.h>

class PulseData
{
public:
	float Strength;
	float Type;
	float Padding1;
	float Padding2;
};

class Pulse
{
public:
	int type;
	float strength;
	float timeLeft;
	float timeMax;

	Pulse(float time)
	{
		this->timeLeft = time;
		this->timeMax = time;
		this->strength = 200;
		this->type = 0;
	}
};

class FireData
{
public:
	DirectX::XMFLOAT4 dir;
};

class PotentialEngine : public SmoothGame::Renderer
{
	std::vector<SmoothGraphics::PlayerData> playerData;
	std::vector<Pulse> pulses;
	
public:
	PotentialEngine(std::vector<SmoothGame::PositionComponent*>);
	HRESULT Init();
};