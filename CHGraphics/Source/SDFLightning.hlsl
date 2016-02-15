Texture3D distTex: register(t2);
Texture2D inputTex: register(t4);
Texture2D colorTex: register(t5);
Texture2D normalTex: register(t6);
RWTexture2D<int> outputTex: register(u7);

SamplerState texSampler;

cbuffer SceneBuffer: register(b0)
{
	matrix projection : PROJECTION;
	matrix view: VIEW;
	float3 eyePosition: EYE;
};

cbuffer ScreenData: register(b11)
{
	matrix projViewInverse;
	float4 screenSize;
}

cbuffer Objects: register(b12)
{
	float4 objects[20];
	uint objectCount;
}

cbuffer LightData: register(b1)
{
	float3 lightPosition;
	float lightIntensity;
}

float sdBox(float3 p, float3 b)
{
	float3 d = abs(p) - b;
	return min(max(d.x, max(d.y, d.z)), 0.0) +
		length(max(d, 0.0));
}

 [numthreads(8, 8, 10)]
void main( uint3 dtId : SV_DispatchThreadID )
{
	if(dtId.z == 0)
		outputTex[dtId.xy] = 100.f;
	float4 ndc = float4(dtId.xy / screenSize.xy, inputTex.SampleLevel(texSampler, dtId.xy / screenSize.xy, 0).x, 1.0f);
	ndc.x = ndc.x * 2.0f - 1.0f;
	ndc.y = -(ndc.y * 2.0f - 1.0f);
	float4 r = mul(projViewInverse, ndc);
	float4 worldPos = r / r.w;

	float3 camPosition = worldPos;
	float3 rS = camPosition;
	float3 rD = normalize(lightPosition - camPosition);
	if (dot(rD, normalTex[dtId.xy].xyz * 2 - 1) < 0.1f)
	{
		return;
	}
	float t = 0.15f;
	int i = 0;
	float4 result = float4(0, 0, 0, 1);
	float shadow = 1.0f;
	for (i = 0; i < 20 && t < 55.0f; ++i)
	{
		float3 p = rS + rD * t;
		float dp = p.y;
		float d = 10000.0f;
		for (uint o = dtId.z; o < objectCount; o += 10)
		{
			float3 op = p;
			op -= objects[o];// + dtId.z * objCount];

			float bD = max(sdBox(op, float3(2,2,2)), 0);
			op.x = (op.x + 2) / 4.0f;
			op.y = (-op.y + 2) / 4.0f;
			op.z = (op.z + 2) / 4.0f;
			float oD = distTex.SampleLevel(texSampler, op, 0).x;
			oD += bD;
			d = min(d, oD);
		}

		if (d < 0.001f)
		{
			shadow = 0;
			break;
		}
		t += d;
		shadow = min(shadow, 30.0f * d/t);
	}

	int old;
	InterlockedMin(outputTex[dtId.xy], shadow * 100, old);
	//outputTex[dtId.xy] = float4(colorTex[dtId.xy].xyz*(shadow + 0.4), 1);;
	//outputTex[dtId.xy] = shadow * 100;//float4(float3(1,1,1) * (i * 0.06f), 1.0f);
}