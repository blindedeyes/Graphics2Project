texture2D thisTexture : register(t0);

SamplerState filters[2] : register(s0);

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
	float3 normal : NORMAL;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	return thisTexture.Sample(filters[0], input.uv);
	return float4(input.uv, 1.0f);
}
