RWTexture2D<float4> Output : register(u0);

Texture2D<float4> Texture : register(t0);
SamplerState g_sampler : register(s0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float2 uv_coordinates;
	uv_coordinates.x = DTid.x / 1920.0f;
	uv_coordinates.y = DTid.y / 1080.0f;

	Output[DTid.xy] = Texture.SampleLevel(g_sampler, uv_coordinates, 0);

}