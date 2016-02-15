struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	uint instanceID : SV_InstanceID;
};

struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float4 diffuseColor: DIFF_COLOR;
	float4 shadowPos: SHADOW_POS;
	float3 position: POSITION;
	float3 normal: NORMAL;
	float3 eyePosition: EYE;
	float metal : METAL;
	float roughness : ROUGHNESS;
};

cbuffer SceneBuffer: register(b0)
{
	matrix projection : PROJECTION;
	matrix view: VIEW;
	float3 eyePosition: EYE;
};

cbuffer ShadowVP: register(b3)
{
	matrix shadowVP: SHADOW_VP;
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
	output.position = pos.xyz;
	output.shadowPos = mul(shadowVP, pos);
	output.shadowPos = output.shadowPos / output.shadowPos.w;
	pos = mul(view, pos);
	pos = mul(projection, pos);
	output.pos = pos;
	output.normal = input.normal;
	output.eyePosition = eyePosition;
	output.diffuseColor = mats[input.instanceID].material.diffuseColor;
	output.metal = mats[input.instanceID].material.properties.x;
	output.roughness = mats[input.instanceID].material.properties.y;
	return output;
}
