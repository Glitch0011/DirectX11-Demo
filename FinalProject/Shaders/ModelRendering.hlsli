struct VsIn
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
};

struct VsOut
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Normal : TEXCOORD1;
	float4 WorldPos : COLOR2;
	uint ID : COLOR3;
}; 

struct GsIn
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Normal : TEXCOORD1;
	float4 WorldPos : COLOR2;
	uint ID : COLOR3;
};

struct GsOut
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Normal : TEXCOORD1;
	float4 WorldPos : COLOR2;
	uint ID : COLOR3;
	uint primID : SV_PrimitiveID;
};

struct PsIn
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Normal : TEXCOORD1;
	float4 WorldPos : COLOR2;
	uint ID : COLOR3;
};

struct PsOut
{
	float4 Colour : COLOR;
	float4 Normal : COLOR1;
	float4 WorldPos : COLOR2;
	float4 VertexData : COLOR3;
};