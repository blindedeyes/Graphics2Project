TextureCube skyBoxTex : register(t0);
SamplerState filter : register(s0);

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 uv : UV;
};

float4 main(PixelShaderInput input) : SV_TARGET
{

    return skyBoxTex.Sample(filter, input.uv);
}