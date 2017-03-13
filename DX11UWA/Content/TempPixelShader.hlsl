
#define directional 1
#define pointlight 2
#define spotlight 3
#define ambient 4

Texture2D thisTexture : register(t0);
Texture2D shadowMap : register(t1);

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
	//Lighting
    for (int i = 0; i < 16; i++)
    {
        float ratio = 0.0;
        if (LightValues[i].pos[3] == 0)
            break;
        float4 dir;
        switch ((int) LightValues[i].pos[3])
        {
            case directional:
                float3 screenpos = input.lPos.xyz / input.lPos.w;
                //on screen check
                if (screenpos.x > 1.0f || screenpos.x < -1.0f ||
                    screenpos.y > 1.0f || screenpos.y < -1.0f ||
                    screenpos.z > 1.0f || screenpos.z < -1.0f)
                {
                //off the range of the shadow map
                //skip this light run.
                    break;
                }
                //put it in uv coords
                screenpos.x = screenpos.x / 2 + 0.5;
                screenpos.y = screenpos.y / -2 + 0.5;
                
                float depth = shadowMap.Sample(filters, screenpos.xy);

                //Ignore this light if its depth is farther than what is in shadow map
                float bias = 0.005;
                if (depth < screenpos.z - bias)
                    break;
                
                ratio = dot(-1 * LightValues[i].dir.xyz, input.normal.xyz);
                ratio = saturate(ratio);
                lightColor += saturate((LightValues[i].color * ratio));
                
                
                break;
            case pointlight:
                dir = LightValues[i].pos;
                dir[3] = 0;
                dir = normalize(dir - input.worldPos);
                ratio = dot(dir.xyz, input.normal);
                ratio = saturate(ratio);
                ratio *= (1.0 - saturate(length(LightValues[i].pos - input.worldPos) / LightValues[i].radius[0]));
                lightColor += saturate((LightValues[i].color * ratio));
                break;
            case spotlight:
                dir = LightValues[i].pos;
                dir[3] = 0;
                dir = float4(normalize(dir.xyz - input.worldPos.xyz), 0);

                float surfaceRatio = saturate(dot((dir.xyz) * -1, normalize(LightValues[i].dir.xyz)));
                float spotFactor = ((surfaceRatio > LightValues[i].radius.z) ? 1 : 0);


                ratio = saturate(dot(dir.xyz, input.normal.xyz));

				//radius code
                ratio = (ratio * spotFactor);
				

				//distance radius
                ratio *= (1.0 - saturate(length(LightValues[i].pos - input.worldPos) / LightValues[i].radius[0]));
                ratio *= (1.0 - saturate((LightValues[i].radius[1] - surfaceRatio) / (LightValues[i].radius[1] - LightValues[i].radius[2])));

                lightColor += ((LightValues[i].color * ratio));

				
                break;
            case ambient:
				//X is the ratio of ambient light
                lightColor += ((LightValues[i].color * LightValues[i].radius.x));

                break;
            default:
                break;
        }
    }

    float4 textureColor = thisTexture.Sample(filters, input.uv.xy);
    textureColor = textureColor * lightColor;
    return saturate(textureColor);
	//return float4(input.uv, 1.0f);
}