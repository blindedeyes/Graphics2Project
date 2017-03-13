// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float3 pos : POSITION;
    float3 uv : UV;
    float3 normal : NORMAL;
    float4 Tangent : TANGENT;
};

// Per-pixel color data passed through the pixel shader.
struct TOGEOSHADER
{
    float4 pos : SV_POSITION;
    float3 uv : UV;
    float3 normal : NORMAL;
    float4 Tangent : TANGENT;
    uint instID : instID;
};

//// Simple shader to do vertex processing on the GPU.
TOGEOSHADER main(VertexShaderInput input, uint instID: SV_InstanceID)
{
    TOGEOSHADER output;
    output.pos = float4(input.pos, 1);
    output.uv = input.uv;
    output.normal = input.normal;
    output.Tangent = input.Tangent;
    output.instID = instID;
    return output;
}
