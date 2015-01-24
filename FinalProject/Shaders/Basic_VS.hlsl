#include <VS_Input.hlsli>

StructuredBuffer<Buff> ParticlesRO : register(t0);

cbuffer VS_ConstantBuffer : register(b0)
{
	matrix View;
	matrix Projection;
}

cbuffer VS_QuickConstantBuffer : register(b1)
{
	matrix World;
}

VsOut main(VsIn input, uint instanceID : SV_InstanceID, uint ID : SV_VertexID)
{
	VsOut output = (VsOut)0;

	float3 rot = ParticlesRO[instanceID].Rotation;
	float4x4 rotationX =
	{
		1, 0, 0, 0,
		0, cos(rot.x), -sin(rot.x), 0,
		0, sin(rot.x), cos(rot.x), 0,
		0, 0, 0, 1,
	};
	float4x4 rotationY = 
	{
		cos(rot.y),  0, sin(rot.y),0,
		0,           1, 0,         0,
		-sin(rot.y), 0, cos(rot.y),0,
		0,           0, 0,         1,
	};
	float4x4 rotationZ =
	{
		cos(rot.z), -sin(rot.z), 0, 0,
		sin(rot.z), cos(rot.z),  0, 0,
		0, 0, 0, 0,
		0, 0, 0, 1,
	};

	float3 sca = ParticlesRO[instanceID].Scale;
	float4x4 scale = 
	{
		sca.x, 0,     0,     0,
		0,     sca.y, 0,     0,
		0,     0,     sca.z, 0,
		0,     0,     0,     1,
	};

	float4 worldPos = mul(input.Pos, rotationX);
	worldPos = mul(worldPos, rotationZ);
	worldPos = mul(worldPos, rotationY);
	worldPos = mul(worldPos, scale);

	float4 pos = ParticlesRO[instanceID].Pos + worldPos;

	output.Pos = pos;

	output.Pos = mul(output.Pos, World);

	output.Pos = mul(output.Pos, View);

	output.Pos = mul(output.Pos, Projection);

	output.col = ParticlesRO[instanceID].Col;

	return output;
}