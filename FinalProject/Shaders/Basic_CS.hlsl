#include <VS_Input.hlsli>
#include <Quadtree.hlsli>

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

#include <GridSystem.hlsli>

float3 Goto(uint id)
{
	float3 vec = movingData[id].Target - positionData[id].Pos;

	float3 vecNorm = normalize(vec);

	float4 accel = float4(0, 0, 0, 0);

	if (length(vec) > 0.1)
		accel.xyz += vecNorm * 0.1;
	
	return accel;
}

float3 Charge(uint id)
{
	float3 vec = movingData[id].Target;

	float3 vecNorm = vec;

	float4 accel = float4(0, 0, 0, 0);

	float3 currentVel = positionData[id].Vel;

	float currentAngle = atan2(currentVel.y, currentVel.x);
	float desiredAngle = atan2(vec.y, vec.x);
	float diff = abs(desiredAngle - currentAngle);

	float strength = 0.1f;
	positionData[id].Vel *= (1 - ((diff / (2 * PI)) * strength));

	accel.xyz += vecNorm * 8.0;

	movingData[id].Padding.x -= time;

	if (movingData[id].Padding.x < 0)
	{
		movingData[id].State = STATE_NONE;
		movingData[id].Player = 0;
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

		float speedFactor = 1.0;

	float l = length(vec);
	if (l > playerData[playerIndex].FollowRadius)
	{
		float distanceOver = max(l - playerData[playerIndex].FollowRadius, 0);
		float strengthMult = min(1 * exp(distanceOver), 10);
		accel.xyz += vecNorm * speedFactor * strengthMult;
	}
	else
	{
		/*float angle = atan2(vec.y, vec.x);
		angle += 0.0174 * 10;
		vecNorm = float3(sin(angle), -cos(angle), 0);

		accel.xyz -= vecNorm * speedFactor;
		accel /= 2;*/

		//accel.xyz -= vecNorm * speedFactor;
	}

	return accel;
}

float2 GetLocalAccel(uint2 pos)
{
	uint index = GetGridIndexFromGridPos(pos);
	if (index > 0 && index < (gridSize.x * gridSize.y) && length(gridData[index].Vel) > 0)
		return normalize(gridData[index].Vel) * (gridData[index].Count * 0.1);
	else
		return 0;
}

float2 GetLocalVector(uint2 pos, uint radius = 1)
{
	float2 total = 0;
	float fullDist = (float)radius * 2.0;

	float2 fPos = pos;
	float fRadius = radius;

	[loop]
	for (int x = -fRadius; x < fRadius + 1; x++)
	{
		[loop]
		for (int y = -fRadius; y < fRadius + 1; y++)
		{
			float2 vect = float2(x, y);
			float factor = 1.0 - (length(vect) / fullDist);

			total += (factor * vect) * GetGrid(fPos + vect);
		}
	}

	total *= -1;

	float len = sqrt((total.x * total.x) + (total.y * total.y));
	if (len == 0)
	{
		return float2(0, 0);
	}
	else
	{
		return total / len;
	}
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

	float3 accel = float3(0, 0, 0);
	unsigned int elementCount = batchSize.x * batchSize.y * batchSize.z * BATCH_SIZE_X * BATCH_SIZE_Y * BATCH_SIZE_Z;

	float2 pos = positionData[id].Pos.xy;

	uint2 uintPos = GetGridPosFromWorldPos(positionData[id].Pos.xy);

	accel.xy += GetLocalVector(uintPos) * 0.001;
	
	if (length(accel) > 0)
		accel = normalize(accel);

	//Now do the calculations
	switch (movingData[id].State)
	{
		//1 = Goto
		case STATE_GOTO:
			accel += Goto(id);
			break;
		case STATE_FOLLOW:
			accel += Follow(id);
			break;
		case STATE_CHARGE:
			accel += Charge(id);
			break;
	}
	if (movingData[id].State == STATE_NONE)
	{
		accel.xy += GetLocalAccel(uintPos) * 1.2;
	}
	else
	{
		accel.xy += GetLocalAccel(uintPos) * 0.1;
	}

	if (length(accel) > 0)
	{
		positionData[id].Vel.xyz += accel;
	}

	if (movingData[id].State == STATE_NONE)
	{
		
		positionData[id].Vel *= 0.999;
	}
	else
	{
		positionData[id].Vel *= 0.99f;
	}
	
	float3 nextPos = positionData[id].Pos + (positionData[id].Vel * time) + (accel * time * time);

	float bounds = 495;

	if (nextPos.x > -bounds &&
		nextPos.y > -bounds &&
		nextPos.x < bounds &&
		nextPos.y < bounds)
	{
		positionData[id].Pos.xyz = nextPos;
	}
	else
	{
		/*movingData[id].Player = 0;
		movingData[id].State = STATE_NONE;
		movingData[id].TargetCol.xyz = HUEtoRGB(0.0 / 360.0);
		movingData[id].TargetCol.w = 0.5;*/
		positionData[id].Vel *= -0.1f;
	}
	
	//positionData[id].Col += normalize(movingData[id].TargetCol - positionData[id].Col) * time;
	//positionData[id].Col = saturate(positionData[id].Col);

	positionData[id].Col.xyz = saturate(GetLocalGrid(uintPos) / ((float)elementCount * 0.001)) * movingData[id].TargetCol;
	positionData[id].Col.xyz += float3(0.1, 0.1, 0.1);
	positionData[id].Col.w = 1;
}