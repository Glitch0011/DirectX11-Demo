#include <ModelRendering.hlsli>

cbuffer VS_ConstantBuffer : register(b0)
{
	matrix View;
	matrix Projection;
	float3 cameraPosition;
}

cbuffer VS_AmbientConstantBuffer : register(b1)
{
	float4 ambient;
	uint modelID;
	bool highlight;
}

cbuffer VS_TextureAvaliability : register(b2)
{
	float textureMapped;
	float bumpMapped;
	float environmentMapped;
	float heightMapped;
};

cbuffer PS_HighlightingData : register(b3)
{
	float4 highlightingData;
};

Texture2D modelTexture : register(t0);
Texture2D environmentTexture : register(t1);
Texture2D bumpTexture : register(t2);
Texture2D heightTexture : register(t3);

SamplerState samplerState;

//http://www.gamedev.net/topic/559871-tangent-space-matrix-from-normal/
float3x3 compute_tangent_frame(float3 N, float3 P, float2 UV)
{
	float3 dp1 = ddx(P);
	float3 dp2 = ddy(P);
	float2 duv1 = ddx(UV);
	float2 duv2 = ddy(UV);

	float3x3 M = float3x3(dp1, dp2, cross(dp1, dp2));
	float2x3 inverseM = float2x3( cross( M[1], M[2] ), cross( M[2], M[0] ) );
	float3 T = mul(float2(duv1.x, duv2.x), inverseM);
	float3 B = mul(float2(duv1.y, duv2.y), inverseM);

	return float3x3(normalize(T), normalize(B), N);
}

static const float PI = 3.14159265359;

PsOut main(PsIn input) : SV_TARGET
{
	PsOut output = (PsOut)0;

	//Pass through data
	output.WorldPos = input.WorldPos;
	output.VertexData = float4(modelID, input.ID, 0, 0);

	if (!highlight || highlightingData.y != input.ID)
	{
		float3 outputNormal;

		float2 parallaxTex = input.TexCoord;

		//Work out normals
		if (bumpMapped)
		{
			//We work out the advance normal
			float3 dir = reflect(-normalize(input.WorldPos - cameraPosition), -input.Normal);
			float3x3 tangentFrame = compute_tangent_frame(input.Normal, dir, input.TexCoord);

			//Work out parallex
			//http://www.d3dcoder.net/Data/Resources/ParallaxOcclusion.pdf
			if (heightMapped)
			{
				float3 toEye = cameraPosition - input.WorldPos;
				toEye = normalize(toEye);
				float3 viewDirW = toEye;

				float3x3 toTangent = transpose(tangentFrame);
				float3 viewDirTS = mul(viewDirW, toTangent);

				float gHeightScale = 0.015f;
				float gMinSampleCount = 8; 
				float gMaxSampleCount = 64;

				float2 maxParallaxOffset = -viewDirTS.xy * gHeightScale / viewDirTS.z;

				float a = dot(toEye, input.Normal);
				int sampleCount = (int)lerp(gMaxSampleCount, gMinSampleCount, a);

				float zStep = 1.0 / (float)sampleCount;
				float2 texStep = maxParallaxOffset * zStep;

				float2 dx = ddx(input.TexCoord);
				float2 dy = ddy(input.TexCoord);

				int sampleIndex = 0;
				float2 currTexOffset = 0, prevTexOffset = 0, finalTexOffset = 0;
				float currRayZ = 1.0f - zStep, prevRayZ = 1.0f, currHeight = 0.0f, prevHeight = 0.0f;

				while (sampleIndex < sampleCount + 1)
				{
					currHeight = heightTexture.SampleGrad(samplerState, input.TexCoord + currTexOffset, dx, dy).r;
					if (currHeight > currRayZ)
					{
						float t = (prevHeight - prevRayZ) / (prevHeight - currHeight + currRayZ - prevRayZ);
						finalTexOffset = prevTexOffset + t * texStep;
						sampleIndex = sampleCount + 1;
					}
					else
					{
						++sampleIndex;

						prevTexOffset = currTexOffset;
						prevRayZ = currRayZ;
						prevHeight = currHeight;
						currTexOffset += texStep;

						currRayZ -= zStep;
					}
				}

				parallaxTex = input.TexCoord + finalTexOffset;
			}

			tangentFrame = compute_tangent_frame(input.Normal, dir, parallaxTex);
			outputNormal = normalize(mul(2.0f * bumpTexture.Sample(samplerState, parallaxTex) - 1.0f, tangentFrame));
		}
		else
		{
			//We pass the simple normal
			outputNormal = -input.Normal;
			//output.Normal.rgb = 0.5f * (input.Normal + 1.0f);
		}

		output.Normal.rgb = 0.5f * (outputNormal + 1.0f);

		//Hide ambient inside the normals A component
		output.Normal.a = ambient;

		//Work out base colour
		float4 colour;
		if (textureMapped == 1.0)
		{
			colour = modelTexture.Sample(samplerState, parallaxTex);
		}
		else
		{
			colour = float4(1, 1, 1, 0);
		}

		//Work out colour from environment map
		if (environmentMapped)
		{
			float3 dir = reflect(-normalize(input.WorldPos - cameraPosition), -outputNormal);

			float2 longlat = float2(atan2(-dir.z, dir.x), acos(dir.y));

			float4 reflection = environmentTexture.Sample(samplerState, longlat / float2(2.0 * PI, PI));

			if (reflection.r == 0 && reflection.g == 0 && reflection.b == 0)
			{
				output.Colour = colour;
			}
			else
			{
				output.Colour = ((reflection * 1.50) + (colour * 0.50)) / 2.0f;
			}
		}
		else
		{
			output.Colour = colour;
		}
	}
	else
	{
		output.Colour = float4(1, 1, 1, 1);
		output.Normal = float4(1, 1, 1, 1);
	}

	return output;
}