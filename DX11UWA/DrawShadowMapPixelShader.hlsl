
#define directional 1
#define pointlight 2
#define spotlight 3
#define ambient 4

//Texture2D thisTexture : register(t0);
Texture2D shadowMap : register(t0);

SamplerState filters : register(s0);

struct Light
{
    float4 pos; // : POSITION;
    float4 dir; // : DIRECTION;
    float4 color; // : COLOR;
    float4 radius; // : RADIUS;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 uv : UV;
    float3 normal : NORMAL;
    float4 worldPos : WORLDPOS;
    float4 Tangent : TANGENT;
    float4 bTangent : BTANGENT;
    float4 lPos : LightPos;

};

cbuffer Lights : register(b0)
{
    Light LightValues[16];
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 lightColor = float4(0, 0, 0, 0);
    input.normal.xyz = normalize(input.normal.xyz);
	

    float depth = shadowMap.Sample(filters, input.uv.xy);
    
    return float4(depth, 0, 0,0);
	//return float4(input.uv, 1.0f);
}