#include <VS_Input.hlsli>

PsOut main(PsIn input) : SV_TARGET
{
	PsOut output;

	output.colour = input.col;

	return output;
}