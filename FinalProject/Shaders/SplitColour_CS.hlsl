#include <VS_Input.hlsli>

RWStructuredBuffer<Buff> ParticlesRW : register(u0);
StructuredBuffer<Buff> ParticlesRO : register(t0);

StructuredBuffer<MovingParticleData> movingData : register(t1);

cbuffer CS_ConstantBuffer : register(b0)
{
	double time;
	double timeTotal;
}

[numthreads(256, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	const unsigned int P_ID = DTid.x;

	float3 position = ParticlesRO[P_ID].Pos.xyz;
	float3 velocity = ParticlesRO[P_ID].Vel.xyz;
	float3 acceleration = movingData[P_ID].Accel.xyz;

	velocity += acceleration * time;
	position += velocity * time;
	velocity *= 0.97f;

	ParticlesRW[P_ID].Pos.xyz = position;
	ParticlesRW[P_ID].Vel.xyz = velocity;

	ParticlesRW[P_ID].Col += normalize(movingData[P_ID].TargetCol - ParticlesRW[P_ID].Col) * 0.5 * time;
	ParticlesRW[P_ID].Rotation += normalize(movingData[P_ID].TargetRotation - ParticlesRW[P_ID].Rotation) * 0.5 * time;
	ParticlesRW[P_ID].Scale += normalize(movingData[P_ID].TargetScale - ParticlesRW[P_ID].Scale) * 0.5 * time;
}