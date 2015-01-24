#include <ModelRendering.hlsli>

cbuffer VS_ConstantBuffer : register(b0)
{
	matrix View;
	matrix Projection;
	float3 cameraPosition;
}

cbuffer VS_AmbientConstantBuffer : register(b1)
{
	float4 ambient;
	uint modelID;
	bool highlight;
}

cbuffer VS_TextureAvaliability : register(b2)
{
	float textureMapped;
	float bumpMapped;
	float environmentMapped;
	float heightMapped;
};

cbuffer PS_HighlightingData : register(b3)
{
	float4 highlightingData;
};

Texture2D modelTexture;

SamplerState samplerState;

PsOut main(VsOut input) : SV_TARGET
{
	PsOut output = (PsOut)0;

	output.Normal.rgb =  0.5f * (input.Normal + 1.0f);
	output.Normal.a = ambient;

	output.WorldPos = input.WorldPos;

	float3 dir = normalize(input.WorldPos - cameraPosition);

	float2 longlat = float2(atan2(dir.z, -dir.x), acos(-dir.y));

	float PI = 3.14159265359;

	output.Colour = float4(modelTexture.Sample(samplerState, longlat / float2(2.0 * PI, PI)).xyz, 0);

	return output;
}