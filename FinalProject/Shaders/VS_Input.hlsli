#define BATCH_SIZE_X 16
#define BATCH_SIZE_Y 16
#define BATCH_SIZE_Z 4

#define STATE_NONE 0
#define STATE_GOTO 1
#define STATE_FOLLOW 2
#define STATE_CHARGE 3

static const float PI = 3.14159265f;

//http://www.chilliant.com/rgb2hsv.html
float3 HUEtoRGB(in float H)
{
	float R = abs(H * 6 - 3) - 1;
	float G = 2 - abs(H * 6 - 2);
	float B = 2 - abs(H * 6 - 4);
	return saturate(float3(R, G, B));
}

float3 HSLtoRGB(in float3 HSL)
{
	float3 RGB = HUEtoRGB(HSL.x);
		float C = (1 - abs(2 * HSL.z - 1)) * HSL.y;
	return (RGB - 0.5) * C + HSL.z;
}

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
	float FollowRadius;
	float3 Padding;
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