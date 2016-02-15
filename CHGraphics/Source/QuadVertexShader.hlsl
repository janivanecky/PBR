struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 texcoord: TEXCOORD;
};

struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float2 texcoord: TEXCOORD;
};

VertexShaderOutput main(VertexShaderInput input) 
{
	VertexShaderOutput output;
	output.pos = float4(input.pos, 1);
	output.texcoord = input.texcoord;
	return output;
}