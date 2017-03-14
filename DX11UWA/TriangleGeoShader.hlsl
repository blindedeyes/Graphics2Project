cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model[16];
    matrix view;
    matrix projection;
    matrix lightView;
    matrix lightProj;
};


// Per-vertex data used as input to the vertex shader.
struct TOGEOSHADER
{
    float4 pos : SV_POSITION;
    float3 uv : UV;
    float3 normal : NORMAL;
    float4 Tangent : TANGENT;
    uint instID : instID;

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

[maxvertexcount(3)]
void main(
	point TOGEOSHADER input[1],
	inout TriangleStream<PixelShaderInput> output
)
{
    PixelShaderInput element[3];
    
    element[0].pos = float4(input[0].pos.xyz, 1);
    element[0].uv = input[0].uv;

    element[1].pos = element[0].pos;
    element[1].uv = input[0].uv;
    
    element[2].pos = element[0].pos;
    element[2].uv = input[0].uv;


    element[0].pos.y += .5f;
    element[1].pos.x += .3f;
    element[2].pos.x -= .3f;
    
    float4 pos = element[0].pos;

    pos = mul(pos, model[input[0].instID]);
    pos = mul(pos, view);
    pos = mul(pos, projection);

    element[0].pos = pos;
/*
    element[0].lPos = input[0].pos;
    element[0].lPos = mul(element[0].lPos, model[input[0].instID]); //mul(model[input[0].instID], mul(lightView, lightProj)));
    element[0].lPos = mul(element[0].lPos, lightView); //mul(model[input[0].instID], mul(lightView, lightProj)));
    element[0].lPos = mul(element[0].lPos, lightProj); //mul(model[input[0].instID], mul(lightView, lightProj)));
*/

    
    pos = element[1].pos;

    pos = mul(pos, model[input[0].instID]);
    pos = mul(pos, view);
    pos = mul(pos, projection);

    element[1].pos = pos;

/*  
    element[1].lPos = input[0].pos;
    element[1].lPos = mul(element[1].lPos, model[input[0].instID]); //mul(model[input[0].instID], mul(lightView, lightProj)));
    element[1].lPos = mul(element[1].lPos, lightView); //mul(model[input[0].instID], mul(lightView, lightProj)));
    element[1].lPos = mul(element[1].lPos, lightProj); //mul(model[input[0].instID], mul(lightView, lightProj)));
*/
    
    pos = element[2].pos;

    pos = mul(pos, model[input[0].instID]);
    pos = mul(pos, view);
    pos = mul(pos, projection);


    element[2].pos = pos;
/*
    element[2].lPos = input[0].pos;
    element[2].lPos = mul(element[2].lPos, model[input[0].instID]); //mul(model[input[0].instID], mul(lightView, lightProj)));
    element[2].lPos = mul(element[2].lPos, lightView); //mul(model[input[0].instID], mul(lightView, lightProj)));
    element[2].lPos = mul(element[2].lPos, lightProj); //mul(model[input[0].instID], mul(lightView, lightProj)));
*/
    output.Append(element[0]);
    output.Append(element[1]);
    output.Append(element[2]);
    
}