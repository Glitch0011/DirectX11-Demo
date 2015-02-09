#include <VS_Input.hlsli>

PsOut main(PsIn input) : SV_TARGET
{
	PsOut output;

	output.colour = input.col;

	output.colour.y = 1;
	output.colour.z = 1;

	return output;
}