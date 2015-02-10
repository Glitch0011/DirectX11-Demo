#include <VS_Input.hlsli>

RWStructuredBuffer<Buff> positionData : register(t0);
RWStructuredBuffer<MovingParticleData> movingData : register(t1);

cbuffer CS_ConstantBuffer : register(b0)
{
	double time;
	double timeTotal;
}

#define GROUP_COUNT 8

[numthreads(16, 16, 4)]
void main(uint3 groupThreadID : SV_GroupThreadID, uint3 groupID : SV_GroupID)
{
	uint3 gUint3 = uint3(GROUP_COUNT, GROUP_COUNT, GROUP_COUNT);
	uint3 tUint3 = uint3(16, 16, 4);

	const unsigned int groupMax = tUint3.x * tUint3.y * tUint3.z;

	const unsigned int subGroupID = (((gUint3.z * groupID.z) * gUint3.y) + (gUint3.x * groupID.y) + groupID.x) * groupMax;

	const unsigned int subThreadID = ((tUint3.x * groupThreadID.x) * tUint3.z) + (groupThreadID.z * tUint3.y) + groupThreadID.y;

	const unsigned int id = subGroupID + subThreadID;

	float3 vec = normalize(movingData[id].Target - positionData[id].Pos) * time * 300;
	if (length(vec) > 0.1)
		positionData[id].Pos.xyz += vec;

	positionData[id].Col += normalize(movingData[id].TargetCol - positionData[id].Col) * time;
}