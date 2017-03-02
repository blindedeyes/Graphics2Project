#define directional 1
#define pointlight 2

texture2D thisTexture : register(t0);

SamplerState filters[2] : register(s0);

struct Light {
	float4 pos;// : POSITION;
	float4 dir;// : DIRECTION;
	float4 color;// : COLOR;
	float4 radius;// : RADIUS;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
	float3 normal: NORMAL;
	float4 worldPos :WORLDPOS;
};

cbuffer Lights : register(b0) {
	Light LightValues[16];
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 lightColor = float4(0,0,0,0);
	//Lighting
	for (int i = 0; i < 16; i++) {
		float ratio = 0.0;
		if (LightValues[i].pos[3] == 0) break;
		switch ((int)LightValues[i].pos[3]) {
			case directional:
				ratio = dot(-1 * LightValues[i].dir, input.normal);
				ratio = saturate(ratio);
				lightColor += saturate((LightValues[i].color * ratio));
				break;
			case pointlight:
				float4 dir = LightValues[i].pos;
				dir[3] = 0;
				dir = normalize(dir - input.worldPos);
				ratio = dot(dir, input.normal);
				ratio = saturate(ratio);
				ratio *= (1.0 - saturate(length(LightValues[i].pos - input.worldPos) / LightValues[i].radius[0]));
				lightColor += saturate((LightValues[i].color * ratio));
				break;
			default:
				break;
		}
	}

	float4 textureColor = thisTexture.Sample(filters[0], input.uv);
	textureColor = textureColor * lightColor;
	return saturate(textureColor);
	//return float4(input.uv, 1.0f);
}
