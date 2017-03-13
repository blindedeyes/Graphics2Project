cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model[5];
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
    element[0].pos.y += .5f;

    element[1].pos = input[0].pos;
    element[1].uv = input[0].uv;
    element[1].pos.x += .3f;
    
    element[2].pos = input[0].pos;
    element[2].uv = input[0].uv;
    element[2].pos.x -= .3f;

    for (int i = 0; i < 3; ++i)
    {
        float4 pos = element[i].pos;

        pos = mul(pos, model[input[0].instID]);
        pos = mul(pos, view);
        pos = mul(pos, projection);

        element[i].pos = pos;
        element[i].lPos = input[0].pos;
        element[i].lPos = mul(element[i].lPos, model[input[0].instID]); //mul(model[input[0].instID], mul(lightView, lightProj)));
        element[i].lPos = mul(element[i].lPos, lightView); //mul(model[input[0].instID], mul(lightView, lightProj)));
        element[i].lPos = mul(element[i].lPos, lightProj); //mul(model[input[0].instID], mul(lightView, lightProj)));

        //float4 temp = float4(element[i].normal, 0);
        
    //normal to world space
        //output.normal = mul(temp, model[instanceID]);
        //output.normal = normalize(output.normal);
    //world pos for lighting
        //output.worldPos = pos;
        //output.pos = pos;
    //uv pass
        //output.uv = input.uv;

    //Normal tangent data
        //output.Tangent = mul(float4(input.Tangent.xyz * input.Tangent.w, 0.0f), model[instanceID]);
        //output.bTangent = mul(float4(cross(input.normal.xyz, input.Tangent.xyz), 0.0f), model[instanceID]);

        output.Append(element[i]);
        //output.Append(element2);
        //output.Append(element3);
    }
        //output.RestartStrip();
}