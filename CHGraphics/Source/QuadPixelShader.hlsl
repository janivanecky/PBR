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

Texture3D distTexture: register(t2);
SamplerState texSampler;

float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 camPosition = eyePosition;
	float3 rS = camPosition;
	float3 z = normalize(-camPosition);
	float3 x = normalize(cross(float3(0,1,0), z));
	float3 y = normalize(cross(z,x));
	float3 screenPos = camPosition + 1 * z + (input.texcoord.x * 2 - 1) * x + (input.texcoord.y * 2 - 1) * y;
	float3 rD = normalize(screenPos - camPosition);
	float t = 0.0f;
	int i = 0;
	float4 result = float4(0,0,0,1);
	for (i = 0; i < 20; ++i)
	{
		float3 p = rS + rD * t;
		float dp = p.y;
		p.x = (p.x + 2) / 4.0f;
		p.y -= 2.0f;
		p.y = (p.y + 2) / 4.0f;
		p.z = (p.z + 2) / 4.0f;
		float d = distTexture.Sample(texSampler, p).x;
		d = min(d, dp);
		if (d < 0.1f)
		{
			break;
		}
		t += d;
	}
	result = float4(float3(1, 1, 1) * (i * 0.05f), 1);
	return result;
}