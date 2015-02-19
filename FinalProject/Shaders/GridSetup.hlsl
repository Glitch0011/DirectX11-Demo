#include <VS_Input.hlsli>
#include <Quadtree.hlsli>

RWStructuredBuffer<Buff> positionData : register(t0);
RWStructuredBuffer<MovingParticleData> movingData : register(t1);

cbuffer CS_ElementData : register(b0)
{
	uint4 batchSize;
}

#include <GridSystem.hlsli>

[numthreads(BATCH_SIZE_X, BATCH_SIZE_Y, BATCH_SIZE_Z)]
void main(uint3 groupThreadID : SV_GroupThreadID, uint3 groupID : SV_GroupID)
{
	//Work out where we are
	uint3 gUint3 = batchSize.xyz;
	uint3 tUint3 = uint3(BATCH_SIZE_X, BATCH_SIZE_Y, BATCH_SIZE_Z);

	const unsigned int groupMax = tUint3.x * tUint3.y * tUint3.z;
	const unsigned int subGroupID = (((gUint3.z * groupID.z) * gUint3.y) + (gUint3.x * groupID.y) + groupID.x) * groupMax;
	const unsigned int subThreadID = ((tUint3.x * groupThreadID.x) * tUint3.z) + (groupThreadID.z * tUint3.y) + groupThreadID.y;

	const unsigned int id = subGroupID + subThreadID;
	
	uint index = GetGridIndexFromGridPos(GetGridPosFromWorldPos(positionData[id].Pos.xy));

	if (movingData[id].Player == 0)
	{
		InterlockedAdd(gridData[index].Count, 1);
	}
	else
	{
		gridData[index].Vel += positionData[id].Vel;
		InterlockedAdd(gridData[index].Count, 10);
	}
}