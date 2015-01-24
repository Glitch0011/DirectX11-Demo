#include <ModelRendering.hlsli>

cbuffer VS_ConstantBuffer : register(b0)
{
	matrix View;
	matrix Projection;
}

cbuffer VS_QuickConstantBuffer : register(b1)
{
	matrix World;
}

VsOut main(VsIn input, uint ID : SV_VertexID)
{
	VsOut output = (VsOut)0;

	output.Pos = mul(float4(input.Pos, 1), World);

	output.WorldPos = output.Pos;

	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	output.TexCoord = input.TexCoord;

	output.Normal = normalize(mul(float4(input.Normal, 0), World));

	output.ID = ID;

	return output;
}