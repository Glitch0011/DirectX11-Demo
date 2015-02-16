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

	accel.xyz += vecNorm * 10.0f;

	movingData[id].Padding.x -= time;

	if (movingData[id].Padding.x < 0)
		movingData[id].State = STATE_FOLLOW;

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

float2 scanTree(int id, int treeIndex, float4 box)
{
	float2 accel = 0;
	float2 pos = positionData[id].Pos.xy;

	if (intersect(quadData[treeIndex].PosAndSize, box))
	{
		for (int i = 0; i < quadData[treeIndex].valueCount; i++)
		{
			Buff _positionalData = positionData[quadData[treeIndex].values[i]];
			MovingParticleData _movingData = movingData[quadData[treeIndex].values[i]];

			if (intersect(box, _positionalData.Pos))
			{
				float2 diff = pos - _positionalData.Pos.xy;
				float l = length(diff);

				float attract = 100;
				float avoid = 50;

				if (l > 0 && l < attract && l > avoid)
				{
					if (any(_positionalData.Vel.xy))
						accel.xy += normalize(_positionalData.Vel.xy) * 2.5;

					if (_movingData.State == STATE_CHARGE && _movingData.Player == 0)
					{
						_movingData.Player = 1;
						_movingData.State = STATE_FOLLOW;
						_movingData.TargetCol.xyz = HUEtoRGB(60.0 / 360.0);
						_movingData.TargetCol.w = 0.5;
					}
				}
				else if (l > 0 && l < avoid)
				{
					accel.xy += normalize(diff) * 10;
				}
			}
		}

		if (quadData[treeIndex].hasChildren)
			for (int i = 0; i < CHILD_COUNT; i++)
				scanTree(id, quadData[treeIndex].children[i], box);
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

	float4 box;
	box.xy = positionData[id].Pos - float2(100, 100);
	box.zw = positionData[id].Pos + float2(100, 100);
	scanTree(id, 0, box);

	/*for (int i = 0; i < elementCount; i++)
	{
		float2 diff = pos - positionData[i].Pos.xy;
		float l = length(diff);

		float attract = 100;
		float avoid = 50;
		if (l > 0 && l < attract && l > avoid)
		{
			if (any(positionData[i].Vel.xy))
				accel.xy += normalize(positionData[i].Vel.xy) * 2.5;

			if (movingData[id].State == STATE_CHARGE && movingData[i].Player == 0)
			{
				movingData[i].Player = 1;
				movingData[i].State = STATE_FOLLOW;
				movingData[i].TargetCol.xyz = HUEtoRGB(60.0 / 360.0);
				movingData[i].TargetCol.w = 0.5;
			}
		}
		else if (l > 0 && l < avoid)
		{
			accel.xy += normalize(diff) * 10;

		}
	}*/

	if (length(accel) > 0)
		accel = normalize(accel) * 5;

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

	if (length(accel) > 0)
	{
		positionData[id].Vel.xyz += accel;
	}

	if (movingData[id].State == STATE_NONE)
	{
		positionData[id].Vel *= 0.80;
	}
	else
	{
		positionData[id].Vel *= 0.975f;// 0.975;
	}
	

	float3 nextPos = positionData[id].Pos + (positionData[id].Vel * time) + (accel * time * time);
	//nextPos += positionData[id].Vel;

	float bounds = 500;

	if (nextPos.x > -bounds && 
		nextPos.y > -bounds && 
		nextPos.x < bounds && 
		nextPos.y < bounds)
	{
		positionData[id].Pos.xyz = nextPos;
	}
	else
	{
		movingData[id].Player = 0;
		movingData[id].State = STATE_NONE;
		movingData[id].TargetCol.xyz = HUEtoRGB(0.0 / 360.0);
		movingData[id].TargetCol.w = 0.5;
		positionData[id].Vel *= -1;
	}
	
	positionData[id].Col += normalize(movingData[id].TargetCol - positionData[id].Col) * time;
	positionData[id].Col = saturate(positionData[id].Col);

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