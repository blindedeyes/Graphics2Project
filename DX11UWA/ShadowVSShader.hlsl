cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model[16];
    matrix view;
    matrix projection;
    matrix lightView;
    matrix lightProj;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float3 pos : POSITION;
    float3 uv : UV;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 pos : SV_POSITION;
};

//// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input, unsigned int instID : SV_InstanceID)
{
    PixelShaderInput output;
    float4 pos = float4(input.pos, 1.0f);

	// Transform the vertex position into projected space.
    pos = mul(pos, model[instID]);
    pos = mul(pos, lightView);
    pos = mul(pos, lightProj);
    //mul(lightView, lightProj)));
    //pos = mul(pos, lightView);
    //pos = mul(pos, projection);
    output.pos = pos;

    return output;
}