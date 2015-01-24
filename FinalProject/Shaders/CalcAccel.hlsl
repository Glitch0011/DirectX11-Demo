#include <VS_Input.hlsli>

StructuredBuffer<Buff> ParticlesRO : register(t0);
RWStructuredBuffer<MovingParticleData> movingData : register(t1);

cbuffer CS_ConstantBuffer : register(b0)
{
	double time;
	double timeTotal;
}

[numthreads(256, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	const unsigned int P_ID = DTid.x;

	float3 accel = float3(0, 0, 0);

	float d = distance(movingData[P_ID].Target, ParticlesRO[P_ID].Pos.xyz);

	if (d > 1.0)
		accel += normalize(movingData[P_ID].Target - ParticlesRO[P_ID].Pos.xyz) * 400;
	else if (d > 0.5)
	{
		movingData[P_ID].Accel = -ParticlesRO[P_ID].Vel / 10.0f;
	}
	else
	{

	}
	
	movingData[P_ID].Accel = accel;
}