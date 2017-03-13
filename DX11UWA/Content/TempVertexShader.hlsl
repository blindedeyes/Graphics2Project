// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
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

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
	float3 normal: NORMAL;
	float4 worldPos : WORLDPOS;
	float4 Tangent : TANGENT;
	float4 bTangent : BTANGENT;
    float4 lPos : LightPos;
};

//// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	output.worldPos = pos;
	output.normal = mul(float4(input.normal,0), model).xyz;
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	// Pass the color through without modification.
	output.uv = input.uv;


	//do later
	//output.normal = input.normal;

	output.Tangent = mul(float4(input.Tangent.xyz * input.Tangent.w, 0.0f), model);
	output.bTangent = mul(float4(cross(input.normal.xyz, input.Tangent.xyz), 0.0f), model);
    output.lPos = mul(float4(input.pos, 1), model);
    output.lPos = mul(output.lPos, lightView);
    output.lPos = mul(output.lPos, lightProj);

	return output;
}
