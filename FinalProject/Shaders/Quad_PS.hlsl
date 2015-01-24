Texture2D colourMap;
Texture2D normalMap;
Texture2D depthMap;

SamplerState samplerState;

struct VsOut
{
	float4 Pos : SV_POSITION;
};

struct LightData
{
	float3 Pos;
	float3 Diffuse;
	float  Range;
	float3 Attr;
	float Angle;
	float3 Colour;
	int Type;
	float3 Direction;
};

cbuffer VS_Quad_ConstantBuffer
{
	int2 screenSize;
	uint lightCount;
	float padding;
}

cbuffer VS_ConstantBuffer
{
	matrix View;
	matrix Projection;
	float3 cameraPosition;
}

StructuredBuffer<LightData> lightData;

//Using float4 rather than PSOut to help memory and branch prediction
float4 main(VsOut input) : SV_TARGET
{
	float2 pos = input.Pos / screenSize;
	
	float4 worldPos = depthMap.Sample(samplerState, pos);

	//If world pos is empty, skip
	if (worldPos.x == 0 || worldPos.y == 0 || worldPos.z == 0)
		return float4(0,0,0,0);

	//Get Normal data
	float4 s = normalMap.Sample(samplerState, pos);

	//Compiler should hopefully inline this
	float ambient = s.w;

	float3 lightStrength = float3(ambient, ambient, ambient);

	if (ambient != 1.0)
	{
		float3 normal = 2.0f * s.xyz - 1.0f;

		float3 eyeToPixelVector = normalize(float4(cameraPosition, 1) - worldPos);

		for (uint i = 0; i < lightCount; i++)
		{
			LightData j = lightData[i];

			float3 lightToWorldPosVector = j.Pos - worldPos;

			float d = length(lightToWorldPosVector);

			if (d > j.Range)
				continue;

			lightToWorldPosVector /= d;

			float diffuseFactor;
			
			if (j.Type == 0)
				diffuseFactor = dot(lightToWorldPosVector, normal);
			else if (j.Type == 1)
				diffuseFactor = pow(max(dot(-lightToWorldPosVector, j.Direction), 0.0f), j.Angle);
			else if (j.Type == 2)
				diffuseFactor = dot(j.Direction, normal);

			float3 Reflect = normalize(2 * diffuseFactor * normal - lightToWorldPosVector);
			float specularFactor = pow(saturate(dot(Reflect, eyeToPixelVector)), 8);

			if (diffuseFactor > 0.0 || specularFactor > 0.0)
			{
				float3 strength = (diffuseFactor * j.Diffuse * j.Colour) + (specularFactor * float3(1, 1, 1));
				lightStrength += strength / (j.Attr[0] + (j.Attr[1] * d) + (j.Attr[2] * (d * d)));
			}
		}

		return saturate(colourMap.Sample(samplerState, pos) * float4(lightStrength, 1));
	}
	else
	{
		//Ambient means just pump the colour out
		return colourMap.Sample(samplerState, pos);
	}
}