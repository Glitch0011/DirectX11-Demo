#include <VS_Input.hlsli>

Texture2D pixelTexture : register(t0);

SamplerState samplerState;

PsOut main(PsIn input) : SV_TARGET
{
	PsOut output;

	output.colour = pixelTexture.Sample(samplerState, input.texCoord) * input.col;

	return output;
}