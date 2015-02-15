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

float3 Goto(uint id)
{
	float3 vec = movingData[id].Target - positionData[id].Pos;

	float3 vecNorm = normalize(vec);

	float4 accel = float4(0, 0, 0, 0);

	if (length(vec) > 0.1)
	{
		accel.xyz += vecNorm * 0.1;
	}

	return accel;
}

float3 Follow(uint id)
{
	float playerIndex = movingData[id].Player - 1;

	float3 vec = playerData[playerIndex].Pos - positionData[id].Pos;
	vec.z = 0;

	float3 vecNorm = normalize(vec);

	float4 accel = float4(0, 0, 0, 0);

	float speedFactor = 0.1;

	if (length(vec) > 200)
	{
		accel.xyz += vecNorm * speedFactor;
	}
	else
	{
		float angle = atan2(vec.y, vec.x);
		angle += 0.0174;
		vecNorm = float3(sin(angle), -cos(angle), 0);

		accel.xyz -= vecNorm * speedFactor;
		accel /= 2;
	}

	return accel;
}

[numthreads(16, 16, 4)]
void main(uint3 groupThreadID : SV_GroupThreadID, uint3 groupID : SV_GroupID)
{
	//Work out where we are
	uint3 gUint3 = batchSize.xyz;
	uint3 tUint3 = uint3(16, 16, 4);

	const unsigned int groupMax = tUint3.x * tUint3.y * tUint3.z;
	const unsigned int subGroupID = (((gUint3.z * groupID.z) * gUint3.y) + (gUint3.x * groupID.y) + groupID.x) * groupMax;
	const unsigned int subThreadID = ((tUint3.x * groupThreadID.x) * tUint3.z) + (groupThreadID.z * tUint3.y) + groupThreadID.y;

	const unsigned int id = subGroupID + subThreadID;

	float3 accel;

	//Now do the calculations
	switch (movingData[id].State)
	{
		//1 = Goto
		case 1:
			accel = Goto(id);
			break;
		case 2:
			accel = Follow(id);
			break;
	}

	positionData[id].Vel.xyz += accel;

	positionData[id].Vel *= 0.99;

	positionData[id].Pos.xyz = positionData[id].Pos + (positionData[id].Vel * time) + (accel * time * time);

	positionData[id].Pos += positionData[id].Vel;
	//positionData[id].Pos.z = 0;

	positionData[id].Col += normalize(movingData[id].TargetCol - positionData[id].Col) * time;

	/*int total = gUint3.x * gUint3.y * gUint3.z + tUint3.x + tUint3.y + tUint3.z;
	float col = 0;
	for (int i = 0; i < total; ++i)
	{
		float l = distance(positionData[id].Pos, positionData[i].Pos);
		if (l < 500)
		{
			col += (500 - l) / (500 * 20);
		}
	}

	positionData[id].Col.xyz = min(col, 1);
	positionData[id].Col.w = 1;*/
}