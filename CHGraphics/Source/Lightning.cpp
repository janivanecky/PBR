#include "Lightning.h"
#include "Core\Math.h"

float RadicalInverse(unsigned int bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(float(bits) * 2.3283064365386963e-10); // / 0x100000000
}

Vector2 Hammersley(int i, int N)
{
	float x1 = (float)i / (float)N;
	float x2 = RadicalInverse(i);
	return Vector2(x1, x2);
}

Vector3 ImportanceSampleH(Vector2 xi, float roughness)
{
	float a = roughness * roughness;
	float Phi = 2 * PI * xi.x;
	float CosTheta = Math::Sqrt((1 - xi.y) / (1 + (a*a - 1) * xi.y));
	float SinTheta = Math::Sqrt(1 - CosTheta * CosTheta);
	Vector3 h;
	h.x = SinTheta * Math::Cos(Phi);
	h.y = CosTheta;
	h.z = SinTheta * Math::Sin(Phi);
	return h;
}

float Shlick(float hl, float metal)
{
	float term = 1 - hl;
	term = term * term * term * term * term;
	float result = metal + (1 - metal) * term;
	return result;
}

float GGX(Vector3 n, Vector3 h, float alpha)
{
	float nm = Math::Dot(h, n);
	float nominator = alpha * alpha;
	float denominator = (nm * nm) * (nominator - 1) + 1;
	denominator *= denominator;
	denominator *= PI;
	return nominator / denominator;
}

float GGXSmith1(Vector3 v, Vector3 n, float alpha)
{
	float NV = Math::Max(Math::Dot(n, v), 0.0f);
	float nominator = (NV);
	float denominator = NV * (1 - alpha) + alpha;
	return nominator / denominator;
}

float GGXSmith(Vector3 l, Vector3 v, Vector3 n, float alpha)
{
	return GGXSmith1(l, n, alpha) * GGXSmith1(v, n, alpha);
}

Vector2 BRDF(Vector3 l, Vector3 v, Vector3 n, float roughness, float metal)
{
	Vector3 h = Math::Normalize(l + v);
	float alpha = roughness * roughness;
	float D = GGX(n, h, alpha);
	float k = (roughness + 1) / 2.0f;
	k *= k;
	float G = GGXSmith(l, v, n, k);
	float hl = Math::Dot(l, h);
	float F = Shlick(hl, metal);
	float Diffuse = (1.0f - F) / PI;
	float dotV = Math::Max(Math::Dot(v, n), 0.0f);
	float dotL = Math::Max(Math::Dot(l, n), 0.0f);
	float Specular = (F * D * G) / (4 * dotV * dotL);
	return Vector2(Diffuse, Specular);
}


Vector2 Lightning::CalculateAmbientBRDF(float NV, float metal, float roughness)
{
	Vector3 v;
	v.x = Math::Sqrt(1.0f - NV * NV); 
	v.y = NV;
	v.z = 0;
	Vector3 n = Vector3(0,1,0);
	float diffuse = 0;
	float specular = 0;
	const uint32 NumSamples = 1000;
	for (uint32 i = 0; i < NumSamples; i++)
	{
		Vector2 Xi = Hammersley(i, NumSamples);
		Vector3 h = ImportanceSampleH(Xi, roughness);
		Vector3 l = 2 * Math::Dot(v, h) * h - v;
		float NoL = Math::Clamp(Math::Dot(n,l), 0, 1);
		float NoH = Math::Clamp(Math::Dot(n,h), 0, 1);
		float VoH = Math::Clamp(Math::Dot(v,h), 0, 1);
		if (NoL > 0)
		{
			float G = GGXSmith(l, v, n, roughness * roughness);//Roughness, NoV, NoL);
			float F = Shlick(VoH, metal);
			float Specular = F * G * VoH / (NoH * NV);
			float Diffuse = ((1 - F) / PI) * VoH;
			diffuse += Diffuse;// * Math::Abs(Math::Sin(NV * PI2 * 2.0f));
			specular += Specular;// * Math::Abs(Math::Sin(NV * PI2 * 2.0f));
		}
	}
	return Vector2(diffuse, specular) / (float)NumSamples;
}