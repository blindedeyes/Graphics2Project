
struct RWData
{
    float3 pos;
    float3 uv;
    float3 normal;
    float4 tangent;
    //uint instID : instID;
};

RWStructuredBuffer<RWData> results : register(u0);

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model[16];
    matrix view;
    matrix projection;
    matrix lightView;
    matrix lightProj;
};


//3d array stuff.
//array of 25 threads.
[numthreads(16, 25, 2)]
void main(uint index : SV_GroupIndex, uint3 gThreadID : SV_GroupThreadID, uint3 stuff : SV_DispatchThreadID)
{
    float3 temp; //= results[index].pos;
    
    temp.x = gThreadID.r;
    temp.y = gThreadID.g;
    temp.z = gThreadID.b;
    //temp.w = 0;
    
    //index = stuff.x;



    //float rand = .01f;
    //results[index].pos = temp.xyz;

    if (results[index].pos.x > model[0]._41 + 12)
        results[index].pos.x = model[0]._41 + 12;
    if (results[index].pos.x < model[0]._41 - 12)
        results[index].pos.x = model[0]._41 + 12;
    results[index].pos.x += results[index].normal.x;
    //rand = noise(index + results[index].pos.x + results[index].pos.y);
    
    if (results[index].pos.y > model[0]._42 + 12)
        results[index].pos.y = model[0]._42 + 12;
    if (results[index].pos.y < model[0]._42 - 12)
        results[index].pos.y = model[0]._42 + 12;
    results[index].pos.y += results[index].normal.y;
    //rand = noise(index + results[index].pos.x + results[index].pos.y);

    
    if (results[index].pos.z > model[0]._43 + 12)
        results[index].pos.z = model[0]._43 + 12;
    if (results[index].pos.z < model[0]._43 - 12)
        results[index].pos.z = model[0]._43 + 12;
    results[index].pos.z += results[index].normal.z;
    //results[index].pos = temp;
    //results[index].uv = (results[index].uv);
    //results[index].normal = float3(0, 0, 0);
    results[index].tangent = float4(0, 0, 0, 0);
    //results[index].instID = index;

}