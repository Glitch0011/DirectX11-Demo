#define BATCH_SIZE_X 16
#define BATCH_SIZE_Y 16
#define BATCH_SIZE_Z 4

struct Buff
{
	float4 Pos;
	float4 Col;
	float4 Vel;
};

struct MovingParticleData
{
	float4 TargetCol;
	float3 Target;
	float State;
	float Player;
	float3 Padding;
};

struct PlayerData
{
	float4 Pos;
};

struct VsIn
{
	float2 Pos : POSITION;
	float2 texCoord : TEXCOORD0;
};

struct VsOut
{
	float4 Pos : SV_POSITION;
	float4 col : COLOR;
	float2 texCoord : TEXCOORD0;
};

struct GsIn
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};

struct GsOut
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};

struct PsIn
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 texCoord : TEXCOORD0;
};

struct PsOut
{
	float4 colour : COLOR;
};