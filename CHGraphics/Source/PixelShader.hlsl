struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 diffuseColor: DIFF_COLOR;
	float3 position: POSITION;
	float3 normal: NORMAL;
	float3 eyePosition: EYE;
	float metal: METAL;
	float roughness: ROUGHNESS;
};

cbuffer LightData: register(b1)
{
	float3 lightPosition;
	float lightIntensity;
};

Texture3D diffTexture : register(t8);
Texture3D specTexture : register(t1);
Texture3D distTexture: register(t2);
SamplerState texSampler;

static const float PI = 3.14156;

float Shlick(float hl, float metal)
{
	float term = 1 - hl;
	term = term * term * term * term * term;
	float result = metal + (1 - metal) * term;
	return result;
}

float GGX(float3 n, float3 h, float alpha)
{
	float nm = dot(h, n);
	float nominator = alpha * alpha;
	float denominator = (nm * nm) * (nominator -  1) + 1;
	denominator *= denominator;
	denominator *= PI;
	return nominator / denominator;
}

float GGXSmith1(float3 v, float3 n, float alpha)
{
	float NV = max(dot(n, v), 0);
	float nominator = (NV);
	float denominator = NV * (1 - alpha) + alpha;
	return nominator / denominator;
}

float GGXSmith(float3 l, float3 v, float3 n, float alpha)
{
	return GGXSmith1(l, n, alpha) * GGXSmith1(v, n, alpha);
}

float4 BRDF(float3 l, float3 v, float3 n, float roughness, float metal, float4 diffuseColor)
{
	float3 h = normalize(l + v);
	float alpha = roughness * roughness;
	float D = GGX(n, h, alpha);
	float k = (roughness + 1) / 2.0f;
	k *= k;
	float G = GGXSmith(l, v, n, k);
	float hl = dot(l, h);
	float F = Shlick(hl, metal);
	float Diffuse = (1.0f - F) / PI;
	float dotV = max(dot(v, n),0);
	float dotL = max(dot(l, n), 0);
	float Specular = (F * D * G) / (4 * dotV * dotL);
	float4 specularColor = float4(1,1,1,1);
	return Diffuse * diffuseColor + Specular * specularColor;
}

struct PIXEL_SHADER_OUTPUT
{
	float4 color: SV_TARGET0;
	float4 normal: SV_TARGET1;
};

PIXEL_SHADER_OUTPUT main(PixelShaderInput input) : SV_TARGET
{
	float4 col = float4(0,0,0,1);
	float3 eyeDir = normalize(input.eyePosition - input.position);
	float3 toLight = lightPosition - input.position;
	float3 lightDir = normalize(toLight);
	float lightDist = dot(toLight, toLight);
	float lDot = clamp(dot(lightDir, input.normal), 0, 1);
	float4 light = PI * float4(1.0f, 1.0f, 1.0f, 1) * lDot * BRDF(lightDir, eyeDir, input.normal, input.roughness, 
																  input.metal, input.diffuseColor) * lightIntensity / lightDist;
	light = saturate(light);
	col += light;
	float4 albedo = float4(0.1f, 0.1f, 0.1f, 1.0f) * .1f;
	float NV = clamp(dot(input.normal, eyeDir), 0, 1);
	float ambientDiff = diffTexture.Sample(texSampler, float3(input.roughness / 4.0f, input.metal, NV)).x * PI;
	float ambientSpec = specTexture.Sample(texSampler, float3(input.roughness / 4.0f, input.metal, NV)).x * PI;
	float4 specColor = float4(1, 1, 1, 1);
	
	float4 ambient = albedo * input.diffuseColor * ambientDiff + albedo * specColor * ambientSpec;
	float3 R = 2 * dot( eyeDir, input.normal) * input.normal - eyeDir;
	float4 result = col + ambient;// * saturate(sin(dot(R, float3(0, 1, 0)) * 5.0f));
	float fog = saturate((length(input.position) - 10.0f) / 30.0f);
	result = float4(pow(result.xyz, 1.0 / 2.2), 1.0f);
	result = lerp(result, float4(1,1,1, 1.0f), fog);
	
	PIXEL_SHADER_OUTPUT output;
	output.color = result;
	output.normal = float4(input.normal * 0.5f + 0.5f, 0.0f);
	return output;
}
