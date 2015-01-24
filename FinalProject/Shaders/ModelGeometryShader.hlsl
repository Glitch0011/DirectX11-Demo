#include <ModelRendering.hlsli>

cbuffer VS_ConstantBuffer
{
	matrix View;
	matrix Projection;
	float3 cameraPosition;
}

[maxvertexcount(3)]
void main(triangle GsIn input[3], uint primID : SV_PrimitiveID, inout TriangleStream<GsOut> output)
{
	if (dot(normalize(input[0].Normal), normalize(input[0].WorldPos - cameraPosition)) < 0.25)
	{
		for (uint i = 0; i < 3; i++)
		{
			GsIn e = input[i];

			GsOut element;

			element.Pos = e.Pos;
			element.TexCoord = e.TexCoord;
			element.Normal = e.Normal;
			element.WorldPos = e.WorldPos;
			element.ID = e.ID;
			element.primID = primID;
				
			output.Append(element);
		}
	}
}