#include <vector>
#include <Renderer.h>

#include <SimpleVertex.h>>
#include <PositionComponent.h>

class PotentialEngine : public SmoothGame::Renderer
{
	std::vector<SmoothGraphics::PlayerData> playerData;
	
public:
	PotentialEngine(std::vector<SmoothGame::PositionComponent*>);
	HRESULT Init();
};