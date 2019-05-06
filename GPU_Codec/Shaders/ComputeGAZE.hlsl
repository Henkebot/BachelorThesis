RWTexture2D<float4> Output : register(u0);

Texture2D<float4> Texture : register(t0);
SamplerState g_sampler : register(s0);

// DCT buffers
StructuredBuffer<float> DCT_Matrix : register(t1);
StructuredBuffer<float> DCT_Matrix_Transpose : register(t2);

#define BLOCK_ROW_COUNT 8
#define BLOCK_SIZE BLOCK_ROW_COUNT* BLOCK_ROW_COUNT

groupshared float4 Pixels[BLOCK_SIZE];

groupshared float3 DCT_MatrixTemp[BLOCK_SIZE];
groupshared float3 DCT_Coefficients[BLOCK_SIZE];

groupshared int YQT[] = {16, 11, 10, 16, 24,  40,  51,  61,  12, 12, 14, 19, 26,  58,  60,  55,
						 14, 13, 16, 24, 40,  57,  69,  56,  14, 17, 22, 29, 51,  87,  80,  62,
						 18, 22, 37, 56, 68,  109, 103, 77,  24, 35, 55, 64, 81,  104, 113, 92,
						 49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99};

groupshared int UVQT[] = {17, 18, 24, 47, 99, 99, 99, 99, 18, 21, 26, 66, 99, 99, 99, 99,
						  24, 26, 56, 99, 99, 99, 99, 99, 47, 66, 99, 99, 99, 99, 99, 99,
						  99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
						  99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99};

cbuffer GazePoint : register(b0)
{
	uint2 gazePos;
	float radius;
	float innerquality;
}

float4 RGBtoYUV(float4 rgba)
{
	float3x3 mYUV709n = {// Normalized
						 0.2126,
						 0.7152,
						 0.0722,
						 -0.1145721060573399,
						 -0.3854278939426601,
						 0.5,
						 0.5,
						 -0.4541529083058166,
						 -0.0458470916941834};

	return float4(mul(mYUV709n, rgba.xyz), 1.0f) + float4(0, 0.5, 0.5, 0);
}

float4 YUVtoRGB(float4 yuva)
{
	float3x3 mYUV709i = {// Inverse Normalized
						 1,
						 0,
						 1.5748,
						 1,
						 -0.187324,
						 -0.468124,
						 1,
						 1.8556,
						 0};
	yuva.gb -= 0.5;
	return float4(mul(mYUV709i, yuva.rgb), yuva.a);
}

uint LinearDistance(int2 Position)
{
	int d = distance(Position, gazePos);

	float d3 = 2000.0f * radius;

	float quality = (1.0f - ((float(d / (d3)))));
	return quality * 85.0f;
}

uint FovDistance(int2 Position)
{
	uint quality, dist;
	float screenDist = 2400.0f * radius;
	float qualityFactor;

	dist = distance(Position, gazePos);

	qualityFactor = (1.0f - (dist / screenDist));

	if(qualityFactor >= 0.9f && qualityFactor <= 1.0f)
	{
		quality = 85;
	}
	else if(qualityFactor >= 0.8f && qualityFactor < 0.9f)
	{
		float num = (0.9f - qualityFactor) / (0.9f - 0.8f);

		quality = lerp(85.0f, 20.0f, num);
	}
	else
	{

		quality = 20;
	}

	return quality;
}

uint CalculateQuality(uint2 Position)
{
	uint quality = 1;
#ifdef LINEAR
	quality = LinearDistance(Position);
#endif

#ifdef FOV
	quality = FovDistance(Position);
#endif

	return quality;
}

[numthreads(BLOCK_ROW_COUNT, BLOCK_ROW_COUNT, 1)] void main(uint3 DispatchThreadID
															: SV_DispatchThreadID, uint3 GroupID
															: SV_GroupID, uint GroupIndex
															: SV_GroupIndex, uint3 GroupThreadID
															: SV_GroupThreadID) {
	int2 coord = GroupID.xy;
	// Divide to get block location
	coord <<= 3;

	uint quality = CalculateQuality(coord);
	quality = (float(quality) * innerquality);

	/*float q = quality / 100.0f;

	Output[DispatchThreadID.xy] = float4(q, 0, 0, 1.0f);
	return;
*/
	quality = quality < 1 ? 1 : quality > 100 ? 100 : quality;
	quality = quality < 50 ? 5000 / quality : 200 - quality * 2;
	int k;

	int yti			= (YQT[GroupIndex] * quality + 50) / 100;
	yti				= (yti < 1 ? 1 : yti > 255 ? 255 : yti);
	YQT[GroupIndex] = yti;


	int uvti		 = (UVQT[GroupIndex] * quality + 50) / 100;
	uvti			 = (uvti < 1 ? 1 : uvti > 255 ? 255 : uvti);
	UVQT[GroupIndex] = uvti;

	coord += GroupThreadID.xy;

	// Convert to uv
	float2 uv_coordinates;
	uv_coordinates.x = coord.x / 1920.0f;
	uv_coordinates.y = coord.y / 1080.0f;

	Pixels[GroupIndex] = (RGBtoYUV(Texture.SampleLevel(g_sampler, uv_coordinates, 0)) * 255.0f) -
						 float4(128.0f, 128.0f, 128.0f, 0.0f);

	GroupMemoryBarrierWithGroupSync();

	DCT_MatrixTemp[GroupIndex] = float3(0, 0, 0);

	[unroll] for(k = 0; k < 8; k++)
	{
		DCT_MatrixTemp[GroupIndex] +=
			DCT_Matrix[GroupThreadID.y * 8 + k] * Pixels[k * 8 + GroupThreadID.x].rgb;
	}

	GroupMemoryBarrierWithGroupSync();

	DCT_Coefficients[GroupIndex] = float3(0, 0, 0);

	[unroll] for(k = 0; k < 8; k++)
	{
		DCT_Coefficients[GroupIndex] +=
			DCT_MatrixTemp[GroupThreadID.y * 8 + k] * DCT_Matrix_Transpose[k * 8 + GroupThreadID.x];
	}
	// Now do we have the coefficients

	GroupMemoryBarrierWithGroupSync();

	DCT_Coefficients[GroupIndex].r =
		round(DCT_Coefficients[GroupIndex].r / YQT[GroupIndex]) * YQT[GroupIndex];
	DCT_Coefficients[GroupIndex].gb =
		round(DCT_Coefficients[GroupIndex].gb / UVQT[GroupIndex]) * UVQT[GroupIndex];

	GroupMemoryBarrierWithGroupSync();

	DCT_MatrixTemp[GroupIndex] = float3(0, 0, 0);

	[unroll] for(k = 0; k < 8; k++)
	{
		DCT_MatrixTemp[GroupIndex] += DCT_Matrix_Transpose[GroupThreadID.y * 8 + k] *
									  DCT_Coefficients[k * 8 + GroupThreadID.x];
	}
	GroupMemoryBarrierWithGroupSync();

	Pixels[GroupIndex] = float4(0, 0, 0, 1.0f);
	[unroll] for(k = 0; k < 8; k++)
	{
		Pixels[GroupIndex].rgb +=
			DCT_MatrixTemp[GroupThreadID.y * 8 + k] * DCT_Matrix[k * 8 + GroupThreadID.x];
	}
	Pixels[GroupIndex].rgb += float3(128.0f, 128.0f, 128.0f);

	Pixels[GroupIndex].rgb /= float3(256.0f, 256.0f, 256.0f);

	Pixels[GroupIndex] = YUVtoRGB(Pixels[GroupIndex]);

	Output[DispatchThreadID.xy] = Pixels[GroupIndex];
}