// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer IntancedModelViewProjectionConstantBuffer : register(b0)
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
    float3 normal : NORMAL;
    float4 Tangent : TANGENT;
};
//
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

//// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input, uint instanceID : SV_InstanceID)
{
    PixelShaderInput output;
    float4 pos = float4(input.pos, 1.0f);
    //pos to proj space
    pos = mul(pos, model[instanceID]);
    output.worldPos = pos;
    pos = mul(pos, view);
    pos = mul(pos, projection);

    float4 temp = float4(input.normal, 0);
    //normal to world space
    output.normal = mul(temp, model[instanceID]);
    output.normal = normalize(output.normal);
    //world pos for lighting
    output.pos = pos;
    //uv pass
    output.uv = input.uv;

    //Normal tangent data
    //output.Tangent = mul(float4(input.Tangent.xyz * input.Tangent.w, 0.0f), model[instanceID]);
    output.lPos = mul(float4(input.pos, 1.0f), model[instanceID]);
    output.lPos = mul(output.lPos, lightView);
    output.lPos = mul(output.lPos, lightProj);
    //output.bTangent = mul(float4(cross(input.normal.xyz, input.Tangent.xyz), 0.0f), model[instanceID]);

    
    output.Tangent = mul(float4(input.Tangent.xyz * input.Tangent.w, 0.0f), model[instanceID]);
    output.bTangent = mul(float4(cross(input.normal.xyz, input.Tangent.xyz), 0.0f), model[instanceID]);

    return output;
}
