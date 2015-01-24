#include <VS_Input.hlsli>

[maxvertexcount(6)]
void main(point GsIn input[1] : SV_POSITION, inout TriangleStream<GsOut> output)
{
	GsOut element;
	element.col = input[0].col;

	element.pos = input[0].pos + float4(-0.25, -0.5, 0, 0);

	output.Append(element);

	element.pos = input[0].pos + float4(-0.25, +0.5, 0, 0);
	output.Append(element);

	element.pos = input[0].pos + float4(+0.25, +0.5, 0, 0);
	output.Append(element);

	output.RestartStrip();

	element.pos = input[0].pos + float4(+0.25, -0.5, 0, 0);
	output.Append(element);

	element.pos = input[0].pos + float4(-0.25, -0.5, 0, 0);
	output.Append(element);

	element.pos = input[0].pos + float4(+0.25, +0.5, 0, 0);
	output.Append(element);
}