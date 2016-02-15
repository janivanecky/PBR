struct PixelShaderInput
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

cbuffer LightData: register(b1)
{
	float3 lightPosition;
	float lightIntensity;
	float3 pointLightPosition;
	float pointLightIntensity;
};

cbuffer ShadowVP: register(b3)
{
	matrix shadowVP: SHADOW_VP;
};

Texture3D diffTexture : register(t8);
Texture3D specTexture : register(t7);
SamplerState texSampler;

Texture2D shadowMap : register(t1);
TextureCube<float> shadowCubeMap : register(t2);

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
	float denominator = (nm * nm) * (nominator - 1) + 1;
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
	float dotV = max(dot(v, n), 0);
	float dotL = max(dot(l, n), 0);
	float Specular = (F * D * G) / (4 * dotV * dotL);
	float4 specularColor = float4(1, 1, 1, 1);
	return Diffuse * diffuseColor + Specular * specularColor;
}

static float2 poissonDisk[16] =
{
	float2(0.2770745f, 0.6951455f),
	float2(0.1874257f, -0.02561589f),
	float2(-0.3381929f, 0.8713168f),
	float2(0.5867746f, 0.1087471f),
	float2(-0.3078699f, 0.188545f),
	float2(0.7993396f, 0.4595091f),
	float2(-0.09242552f, 0.5260149f),
	float2(0.3657553f, -0.5329605f),
	float2(-0.3829718f, -0.2476171f),
	float2(-0.01085108f, -0.6966301f),
	float2(0.8404155f, -0.3543923f),
	float2(-0.5186161f, -0.7624033f),
	float2(-0.8135794f, 0.2328489f),
	float2(-0.784665f, -0.2434929f),
	float2(0.9920505f, 0.0855163f),
	float2(-0.687256f, 0.6711345f)
};

float4 main(PixelShaderInput input) : SV_TARGET
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

	float3 toLightPoint = pointLightPosition - input.position;
	float3 lightDirPoint = normalize(toLightPoint);
	float lightDistPoint = dot(toLightPoint, toLightPoint);
	float lDotPoint = clamp(dot(lightDirPoint, input.normal), 0, 1);
	float4 lightPoint = PI * float4(1.0f, 1.0f, 1.0f, 1) * lDotPoint * BRDF(lightDirPoint, eyeDir, input.normal, input.roughness,
																  input.metal, input.diffuseColor) * pointLightIntensity / lightDistPoint;
	lightPoint = saturate(lightPoint);

	float4 shadowPos = mul(shadowVP, float4(input.position, 1.0f));
	shadowPos /= shadowPos.w;
	float2 shadowTexCoord = float2(shadowPos.x * 0.5f + 0.5f, -shadowPos.y * 0.5f + 0.5f);
	float fromCameraZ = shadowPos.z;
	float shadow = 1.0f;
	if (shadowTexCoord.x >= 0.0f && shadowTexCoord.x <= 1.0f && shadowTexCoord.y >= 0.0f && shadowTexCoord.y <= 1.0f)
	{
		for (int i = 0; i < 16; i++)
		{
			float fromLightZ = shadowMap.Sample(texSampler, shadowTexCoord + poissonDisk[i] / 400.0f).r;
			if (fromLightZ < fromCameraZ - 0.001f)
			{
				shadow -= 1.0f / 16.0f;
			}
		}
	}

	float3 fromLight = input.position - pointLightPosition;

	float current = dot(fromLight, fromLight);
	float3 x = normalize(cross(float3(0, 1, 0), normalize(fromLight)));
	float3 y = normalize(cross(normalize(fromLight), x));
	float val = 1.0f / 16.0f;//clamp(50.0f / (current), 0, 1.0f/ 16.0f);
	float dist = length(fromLight);
	float shadow2 = 1.0f;
	for (int i = 0; i < 16; i++)
	{
		float3 vLightDirection = fromLight + x * poissonDisk[i].x / 3.0f + y * poissonDisk[i].y / 3.0f;
		float4 old = shadowCubeMap.Sample(texSampler, normalize(vLightDirection)).r;
		shadow2 -= (old < current * 0.7f) ? val : 0.0f;
	}
	shadow2 = saturate(shadow2);

	col += light * shadow + lightPoint * shadow2;
	float4 albedo = float4(0.1f, 0.1f, 0.1f, 1.0f) * .02f;
	float NV = clamp(dot(input.normal, eyeDir), 0, 1);
	float ambientDiff = diffTexture.Sample(texSampler, float3(input.roughness / 4.0f, input.metal, NV)).x * PI;
	float ambientSpec = specTexture.Sample(texSampler, float3(input.roughness / 4.0f, input.metal, NV)).x * PI;
	float4 specColor = float4(1, 1, 1, 1);

	float4 ambient = albedo * input.diffuseColor * ambientDiff + albedo * specColor * ambientSpec;
	float3 R = 2 * dot(eyeDir, input.normal) * input.normal - eyeDir;
	float4 result = col + ambient;
	float fog = saturate((length(input.position) - 60.0f) / 10.0f);
	result = float4(pow(result.xyz, 1.0 / 2.2), 1.0f);
	result = lerp(result, float4(1,1,1, 1.0f), fog);
	result.a = input.diffuseColor.a;
	return result;
}
