struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texcoord: TEXCOORD;
};

Texture2D mainTex: register(t0);
SamplerState texSampler;

float4 main(PixelShaderInput input) : SV_TARGET
{
	return mainTex.Sample(texSampler, input.texcoord);
}