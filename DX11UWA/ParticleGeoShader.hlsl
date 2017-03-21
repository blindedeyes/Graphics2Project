cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model[16];
    matrix view;
    matrix projection;
    matrix lightView;
    matrix lightProj;
};


// Per-vertex data used as input to the vertex shader.
struct inData
{
    float3 pos;
    float3 uv;
    float3 normal;
    float4 tangent;
    //uint instID : instID;
};

struct TOGEOSHADER
{
    float4 pos : SV_POSITION;
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

StructuredBuffer<inData> sharedData : register(t0);

[maxvertexcount(3)]
void main(
	point TOGEOSHADER input[1],
	inout TriangleStream<PixelShaderInput> output, uint j : SV_PrimitiveID
)
{
    int i = j;

    PixelShaderInput element[3];
    
    float4 pos = float4(sharedData[i].pos.xyz, 1);

    pos = mul(pos, model[0]);
    pos = mul(pos, view);
    pos = mul(pos, projection);

    if (pos.x / pos.w > -1.2f && pos.x / pos.w < 1.2f)
    if (pos.y / pos.w > -1.2f && pos.y / pos.w < 1.2f)
    
    //if (pos.z > 0.0f && pos.z < 1.2f)
    {
        element[0].pos = pos;
        element[0].uv = sharedData[i].uv;
        element[0].normal = float3(0, 0, 0);

        element[0].worldPos = float4(0, 0, 0, 0);

        element[0].Tangent = float4(0, 0, 0, 0);

        element[0].bTangent = float4(0, 0, 0, 0);

        element[0].lPos = float4(0, 0, 0, 0);

        element[1] = element[0];
        element[2] = element[0];
    

        element[0].normal = float3(0, 0, -1);
        element[0].pos.y += .25f;
        element[1].pos.x += .15f;
        element[2].pos.x -= .15f;
    

        element[2].normal = float3(0, 0, -1);
        element[2].normal = float3(0, 0, -1);

        output.Append(element[0]);
        output.Append(element[1]);
        output.Append(element[2]);
        //output.RestartStrip();
    }
}