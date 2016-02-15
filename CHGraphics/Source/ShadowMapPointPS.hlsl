struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float3 position: POSITION;
};

cbuffer SceneBuffer: register(b0)
{
	matrix projection : PROJECTION;
	matrix view: VIEW;
	float3 eyePosition: EYE;
};


float main(VertexShaderOutput input) : SV_TARGET
{
	float3 dist = input.position - eyePosition;
	return dot(dist, dist);
}