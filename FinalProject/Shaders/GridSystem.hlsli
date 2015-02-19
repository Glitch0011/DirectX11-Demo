struct GridPoint
{
	uint Count;
	float2 Vel;
};

RWStructuredBuffer<GridPoint> gridData : register(t2);

cbuffer CS_ScreenData : register(b2)
{
	uint2 screenSize;
	uint2 gridSize;
}

uint GetGridIndexFromGridPos(uint2 pos)
{
	return (gridSize.x * pos.y) + pos.x;
}

uint GetGrid(uint2 pos)
{
	uint index = GetGridIndexFromGridPos(pos);
	if (index > 0 && index < (gridSize.x * gridSize.y))
		return gridData[index].Count;
	else
		return 0;
}

uint GetLocalGrid(uint2 pos)
{
	return
		GetGrid(pos + uint2(+0, +0)) +

		GetGrid(pos + uint2(+1, +0)) +
		GetGrid(pos + uint2(-1, +0)) +
		GetGrid(pos + uint2(+0, +1)) +
		GetGrid(pos + uint2(+0, -1)) +

		GetGrid(pos + uint2(-1, +1)) +
		GetGrid(pos + uint2(-1, -1)) +
		GetGrid(pos + uint2(+1, +1)) +
		GetGrid(pos + uint2(+1, -1));
}

uint2 GetGridPosFromWorldPos(float2 pos)
{
	float2 _pos = ((pos.xy + uint2(500, 500)) / 1000.0) * gridSize;
	return round(_pos);
}