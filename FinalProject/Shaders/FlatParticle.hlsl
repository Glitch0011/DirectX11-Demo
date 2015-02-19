#include <VS_Input.hlsli>

Texture2D pixelTexture : register(t0);

SamplerState samplerState;

cbuffer CS_ElementData : register(b0)
{
	uint4 batchSize;
}

#include <GridSystem.hlsli>

PsOut main(PsIn input) : SV_TARGET
{
	PsOut output;

	unsigned int elementCount = batchSize.x * batchSize.y * batchSize.z * BATCH_SIZE_X * BATCH_SIZE_Y * BATCH_SIZE_Z;

	float2 a = input.texCoord * gridSize;
	uint2 uintPos = a;
	float3 colour = saturate(GetLocalGrid(uintPos) / ((float)elementCount * 0.001));

	output.colour = (0.1 * pixelTexture.Sample(samplerState, input.texCoord) * input.col) * (float4(colour, 1) * 0.9);
	output.colour.xyz += float3(0.1, 0.1, 0.1);
	
	return output;
}