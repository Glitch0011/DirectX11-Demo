struct Buff
{
	float4 Pos;
	float3 Rotation;
	float3 Scale;
	float4 Col;
	float4 Vel;
};

struct MovingParticleData
{
	float3 Accel;
	float3 Target;
	float4 TargetCol;
	float3 TargetRotation;
	float3 TargetScale;
};

struct VsIn
{
	float4 Pos : POSITION;
	uint instanceID : SV_InstanceID;
	uint ID : SV_VertexID;
};

struct VsOut
{
	float4 Pos : SV_POSITION;
	float4 col : COLOR;
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
};

struct PsOut
{
	float4 colour : COLOR;
};