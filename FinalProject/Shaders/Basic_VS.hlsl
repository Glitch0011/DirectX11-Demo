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

	output.Pos.xyzw = 0;

	output.Pos.xy += input.Pos;

	output.Pos = mul(output.Pos, World);

	output.Pos += ParticlesRO[instanceID].Pos;

	output.Pos = mul(output.Pos, View);

	output.Pos = mul(output.Pos, Projection);

	output.Pos.zw = 1;

	output.texCoord = input.texCoord;

	output.col = ParticlesRO[instanceID].Col;

	return output;
}