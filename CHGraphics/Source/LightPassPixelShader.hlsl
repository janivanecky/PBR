struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texcoord: TEXCOORD;
};

cbuffer SceneBuffer: register(b0)
{
	matrix projection : PROJECTION;
	matrix view: VIEW;
	float3 eyePosition: EYE;
};

Texture2D colBuffer: register(t5);
Texture2D<int> shadowBuffer: register(t7);
SamplerState texSampler;

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 result = colBuffer.Sample(texSampler, input.texcoord);
	int2 texpos = int2(input.texcoord.x * 800, input.texcoord.y * 600);
	int val = shadowBuffer[texpos];
	result *= clamp((float)val / 100.0f + 0.4f, 0, 1);
	return result;
}