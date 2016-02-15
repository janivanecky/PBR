struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	uint instanceID : SV_InstanceID;
};

struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
};

cbuffer SceneBuffer: register(b0)
{
	matrix projection : PROJECTION;
	matrix view: VIEW;
	float3 eyePosition: EYE;
};

struct Material
{
	float4 diffuseColor;
	// x - metal, y - roughness
	float4 properties;
};

struct ModelData
{
	Material material;
	matrix modelMatrix;
};

cbuffer MaterialInstanced: register(b2)
{
	ModelData mats[600];
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	float4 pos = float4(input.pos, 1.0f);
	pos = mul(mats[input.instanceID].modelMatrix, pos);
	pos = mul(view, pos);
	pos = mul(projection, pos);
	output.pos = pos;
	return output;
}
