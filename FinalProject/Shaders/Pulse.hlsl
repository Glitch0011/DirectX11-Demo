#include <VS_Input.hlsli>

RWStructuredBuffer<Buff> positionData : register(t0);
RWStructuredBuffer<MovingParticleData> movingData : register(t1);
RWStructuredBuffer<PlayerData> playerData : register(t2);

cbuffer CS_ConstantBuffer : register(b0)
{
	double time;
	double timeTotal;
}

cbuffer CS_ElementData : register(b1)
{
	uint4 batchSize;
}

cbuffer CS_PulseData : register(b2)
{
	float Strength;
	float Type;
	float StrengthMax;
	float Padding2;
}

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

	if (length(positionData[id].Pos - playerData[0].Pos) < Strength)
	{
		movingData[id].Player = 1;
		movingData[id].State = 2;
		movingData[id].TargetCol.xyz = HUEtoRGB(120.0 / 360.0) * (Strength / StrengthMax);
		movingData[id].TargetCol.w = 0.9;
	}
}