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

class ScreenData
{
public:
	DirectX::XMUINT2 screenSize;
	DirectX::XMUINT2 gridSize;
};

class FireData
{
public:
	DirectX::XMFLOAT4 dir;
};

class GridPoint
{
public:
	unsigned int count;
	DirectX::XMFLOAT2 vel;
};

class PotentialEngine : public SmoothGame::Renderer
{
	std::vector<SmoothGraphics::PlayerData> playerData;
	std::vector<Pulse> pulses;
	int grid_size = 500;

public:
	PotentialEngine(std::vector<SmoothGame::PositionComponent*>);
	HRESULT Init();
};