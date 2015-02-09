#include <VS_Input.hlsli>

RWStructuredBuffer<Buff> positionData : register(t0);
RWStructuredBuffer<MovingParticleData> movingData : register(t1);

cbuffer CS_ConstantBuffer : register(b0)
{
	double time;
	double timeTotal;
}

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const unsigned int id = ((DTid.x) + (DTid.y) + DTid.z);

	float3 vec = normalize(movingData[id].Target - positionData[id].Pos) * time * 10;
	if (length(vec) > 0)
		positionData[id].Pos.xyz += vec;

	positionData[id].Col += normalize(movingData[id].TargetCol - positionData[id].Col) * time;
}