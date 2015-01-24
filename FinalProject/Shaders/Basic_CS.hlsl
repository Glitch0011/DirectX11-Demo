RWStructuredBuffer<float3> positions;

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	int id = DTid.x + DTid.y + DTid.z;

	positions[id].x = positions[id].x * 0.1f;
}