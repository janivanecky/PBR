struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texcoord: TEXCOORD;
};

cbuffer ScreenBuffer: register(b10)
{
	float4 invScreenSize: SIZE;
}

Texture2D mainTex: register(t0);
SamplerState texSampler;

#define FXAA_HLSL_4 1

#ifndef     FXAA_GLSL_120
#define FXAA_GLSL_120 0
#endif
#ifndef     FXAA_GLSL_130
#define FXAA_GLSL_130 0
#endif
#ifndef     FXAA_HLSL_3
#define FXAA_HLSL_3 0
#endif
#ifndef     FXAA_HLSL_4
#define FXAA_HLSL_4 0
#endif    
/*--------------------------------------------------------------------------*/
#if FXAA_GLSL_120
// Requires,
//  #version 120
//  #extension GL_EXT_gpu_shader4 : enable
#define int2 ivec2
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define FxaaBool3 bvec3
#define FxaaInt2 ivec2
#define FxaaFloat2 vec2
#define FxaaFloat3 vec3
#define FxaaFloat4 vec4
#define FxaaBool2Float(a) mix(0.0, 1.0, (a))
#define FxaaPow3(x, y) pow(x, y)
#define FxaaSel3(f, t, b) mix((f), (t), (b))
#define FxaaTex sampler2D
#endif
/*--------------------------------------------------------------------------*/
#if FXAA_GLSL_130
// Requires "#version 130" or better
#define int2 ivec2
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define FxaaBool3 bvec3
#define FxaaInt2 ivec2
#define FxaaFloat2 vec2
#define FxaaFloat3 vec3
#define FxaaFloat4 vec4
#define FxaaBool2Float(a) mix(0.0, 1.0, (a))
#define FxaaPow3(x, y) pow(x, y)
#define FxaaSel3(f, t, b) mix((f), (t), (b))
#define FxaaTex sampler2D
#endif
/*--------------------------------------------------------------------------*/
#if FXAA_HLSL_3
#define int2 float2
#define FxaaInt2 float2
#define FxaaFloat2 float2
#define FxaaFloat3 float3
#define FxaaFloat4 float4
#define FxaaBool2Float(a) (a)
#define FxaaPow3(x, y) pow(x, y)
#define FxaaSel3(f, t, b) ((f)*(!b) + (t)*(b))
#define FxaaTex sampler2D
#endif
/*--------------------------------------------------------------------------*/
#if FXAA_HLSL_4
#define FxaaInt2 int2
#define FxaaFloat2 float2
#define FxaaFloat3 float3
#define FxaaFloat4 float4
#define FxaaBool2Float(a) (a)
#define FxaaPow3(x, y) pow(x, y)
#define FxaaSel3(f, t, b) ((f)*(!b) + (t)*(b))
struct FxaaTex { SamplerState smpl; Texture2D tex; };
#endif
/*--------------------------------------------------------------------------*/
#define FxaaToFloat3(a) FxaaFloat3((a), (a), (a))
/*--------------------------------------------------------------------------*/
float4 FxaaTexLod0(FxaaTex tex, float2 pos)
{
#if FXAA_GLSL_120
	return texture2DLod(tex, pos.xy, 0.0);
#endif
#if FXAA_GLSL_130
	return textureLod(tex, pos.xy, 0.0);
#endif
#if FXAA_HLSL_3
	return tex2Dlod(tex, float4(pos.xy, 0.0, 0.0));
#endif
#if FXAA_HLSL_4
	return tex.tex.SampleLevel(tex.smpl, pos.xy, 0.0);
#endif
}
/*--------------------------------------------------------------------------*/
float4 FxaaTexGrad(FxaaTex tex, float2 pos, float2 grad)
{
#if FXAA_GLSL_120
	return texture2DGrad(tex, pos.xy, grad, grad);
#endif
#if FXAA_GLSL_130
	return textureGrad(tex, pos.xy, grad, grad);
#endif
#if FXAA_HLSL_3
	return tex2Dgrad(tex, pos.xy, grad, grad);
#endif
#if FXAA_HLSL_4
	return tex.tex.SampleGrad(tex.smpl, pos.xy, grad, grad);
#endif
}
/*--------------------------------------------------------------------------*/
float4 FxaaTexOff(FxaaTex tex, float2 pos, int2 off, float2 rcpFrame)
{
#if FXAA_GLSL_120
	return texture2DLodOffset(tex, pos.xy, 0.0, off.xy);
#endif
#if FXAA_GLSL_130
	return textureLodOffset(tex, pos.xy, 0.0, off.xy);
#endif
#if FXAA_HLSL_3
	return tex2Dlod(tex, float4(pos.xy + (off * rcpFrame), 0, 0));
#endif
#if FXAA_HLSL_4
	return tex.tex.SampleLevel(tex.smpl, pos.xy, 0.0, off.xy);
#endif
}

#define FXAA_SRGB_ROP 0

#ifndef     FXAA_DEBUG_PASSTHROUGH
#define FXAA_DEBUG_PASSTHROUGH 0
#endif    
#ifndef     FXAA_DEBUG_HORZVERT
#define FXAA_DEBUG_HORZVERT    0
#endif    
#ifndef     FXAA_DEBUG_PAIR   
#define FXAA_DEBUG_PAIR        0
#endif    
#ifndef     FXAA_DEBUG_NEGPOS
#define FXAA_DEBUG_NEGPOS      0
#endif
#ifndef     FXAA_DEBUG_OFFSET
#define FXAA_DEBUG_OFFSET      0
#endif    
/*--------------------------------------------------------------------------*/
#if FXAA_DEBUG_PASSTHROUGH || FXAA_DEBUG_HORZVERT || FXAA_DEBUG_PAIR
#define FXAA_DEBUG 1
#endif    
#if FXAA_DEBUG_NEGPOS || FXAA_DEBUG_OFFSET
#define FXAA_DEBUG 1
#endif
#ifndef FXAA_DEBUG
#define FXAA_DEBUG 0
#endif

#ifndef FXAA_PRESET
#define FXAA_PRESET 3
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 0)
#define FXAA_EDGE_THRESHOLD      (1.0/4.0)
#define FXAA_EDGE_THRESHOLD_MIN  (1.0/12.0)
#define FXAA_SEARCH_STEPS        2
#define FXAA_SEARCH_ACCELERATION 4
#define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
#define FXAA_SUBPIX              1
#define FXAA_SUBPIX_FASTER       1
#define FXAA_SUBPIX_CAP          (2.0/3.0)
#define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 1)
#define FXAA_EDGE_THRESHOLD      (1.0/8.0)
#define FXAA_EDGE_THRESHOLD_MIN  (1.0/16.0)
#define FXAA_SEARCH_STEPS        4
#define FXAA_SEARCH_ACCELERATION 3
#define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
#define FXAA_SUBPIX              1
#define FXAA_SUBPIX_FASTER       0
#define FXAA_SUBPIX_CAP          (3.0/4.0)
#define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 2)
#define FXAA_EDGE_THRESHOLD      (1.0/8.0)
#define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
#define FXAA_SEARCH_STEPS        8
#define FXAA_SEARCH_ACCELERATION 2
#define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
#define FXAA_SUBPIX              1
#define FXAA_SUBPIX_FASTER       0
#define FXAA_SUBPIX_CAP          (3.0/4.0)
#define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 3)
#define FXAA_EDGE_THRESHOLD      (1.0/8.0)
#define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
#define FXAA_SEARCH_STEPS        16
#define FXAA_SEARCH_ACCELERATION 1
#define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
#define FXAA_SUBPIX              1
#define FXAA_SUBPIX_FASTER       0
#define FXAA_SUBPIX_CAP          (3.0/4.0)
#define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 4)
#define FXAA_EDGE_THRESHOLD      (1.0/8.0)
#define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
#define FXAA_SEARCH_STEPS        24
#define FXAA_SEARCH_ACCELERATION 1
#define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
#define FXAA_SUBPIX              1
#define FXAA_SUBPIX_FASTER       0
#define FXAA_SUBPIX_CAP          (3.0/4.0)
#define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#if (FXAA_PRESET == 5)
#define FXAA_EDGE_THRESHOLD      (1.0/8.0)
#define FXAA_EDGE_THRESHOLD_MIN  (1.0/24.0)
#define FXAA_SEARCH_STEPS        32
#define FXAA_SEARCH_ACCELERATION 1
#define FXAA_SEARCH_THRESHOLD    (1.0/4.0)
#define FXAA_SUBPIX              1
#define FXAA_SUBPIX_FASTER       0
#define FXAA_SUBPIX_CAP          (3.0/4.0)
#define FXAA_SUBPIX_TRIM         (1.0/4.0)
#endif
/*--------------------------------------------------------------------------*/
#define FXAA_SUBPIX_TRIM_SCALE (1.0/(1.0 - FXAA_SUBPIX_TRIM))

float FxaaLuma(float3 rgb)
{
	return rgb.y * (0.587 / 0.299) + rgb.x;
}
/*--------------------------------------------------------------------------*/
float3 FxaaLerp3(float3 a, float3 b, float amountOfA)
{
	return (FxaaToFloat3(-amountOfA) * b) +
		((a * FxaaToFloat3(amountOfA)) + b);
}

float3 FxaaFilterReturn(float3 rgb)
{
#if FXAA_SRGB_ROP
	// Do sRGB encoded value to linear conversion.
	return FxaaSel3(
		rgb * FxaaToFloat3(1.0 / 12.92),
		FxaaPow3(
			rgb * FxaaToFloat3(1.0 / 1.055) + FxaaToFloat3(0.055 / 1.055),
			FxaaToFloat3(2.4)),
		rgb > FxaaToFloat3(0.04045));
#else
	return rgb;
#endif
}

float3 FxaaPixelShader(
	float2 pos,
	FxaaTex tex,
	float2 rcpFrame)
{

	float3 rgbN = FxaaTexOff(tex, pos.xy, FxaaInt2(0, -1), rcpFrame).xyz;
	float3 rgbW = FxaaTexOff(tex, pos.xy, FxaaInt2(-1, 0), rcpFrame).xyz;
	float3 rgbM = FxaaTexOff(tex, pos.xy, FxaaInt2(0, 0), rcpFrame).xyz;
	float3 rgbE = FxaaTexOff(tex, pos.xy, FxaaInt2(1, 0), rcpFrame).xyz;
	float3 rgbS = FxaaTexOff(tex, pos.xy, FxaaInt2(0, 1), rcpFrame).xyz;
	float lumaN = FxaaLuma(rgbN);
	float lumaW = FxaaLuma(rgbW);
	float lumaM = FxaaLuma(rgbM);
	float lumaE = FxaaLuma(rgbE);
	float lumaS = FxaaLuma(rgbS);
	float rangeMin = min(lumaM, min(min(lumaN, lumaW), min(lumaS, lumaE)));
	float rangeMax = max(lumaM, max(max(lumaN, lumaW), max(lumaS, lumaE)));
	float range = rangeMax - rangeMin;
#if FXAA_DEBUG
	float lumaO = lumaM / (1.0 + (0.587 / 0.299));
#endif        
	if (range < max(FXAA_EDGE_THRESHOLD_MIN, rangeMax * FXAA_EDGE_THRESHOLD))
	{
#if FXAA_DEBUG
		return FxaaFilterReturn(FxaaToFloat3(lumaO));
#endif
		return FxaaFilterReturn(rgbM);
	}
#if FXAA_SUBPIX > 0
#if FXAA_SUBPIX_FASTER
	float3 rgbL = (rgbN + rgbW + rgbE + rgbS + rgbM) *
		FxaaToFloat3(1.0 / 5.0);
#else
	float3 rgbL = rgbN + rgbW + rgbM + rgbE + rgbS;
#endif
#endif        

#if FXAA_SUBPIX != 0
	float lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25;
	float rangeL = abs(lumaL - lumaM);
#endif        
#if FXAA_SUBPIX == 1
	float blendL = max(0.0,
					   (rangeL / range) - FXAA_SUBPIX_TRIM) * FXAA_SUBPIX_TRIM_SCALE;
	blendL = min(FXAA_SUBPIX_CAP, blendL);
#endif
#if FXAA_SUBPIX == 2
	float blendL = rangeL / range;
#endif
#if FXAA_DEBUG_PASSTHROUGH
#if FXAA_SUBPIX == 0
	float blendL = 0.0;
#endif
	return FxaaFilterReturn(
		FxaaFloat3(1.0, blendL / FXAA_SUBPIX_CAP, 0.0));
#endif    

	float3 rgbNW = FxaaTexOff(tex, pos.xy, FxaaInt2(-1, -1), rcpFrame).xyz;
	float3 rgbNE = FxaaTexOff(tex, pos.xy, FxaaInt2(1, -1), rcpFrame).xyz;
	float3 rgbSW = FxaaTexOff(tex, pos.xy, FxaaInt2(-1, 1), rcpFrame).xyz;
	float3 rgbSE = FxaaTexOff(tex, pos.xy, FxaaInt2(1, 1), rcpFrame).xyz;
#if (FXAA_SUBPIX_FASTER == 0) && (FXAA_SUBPIX > 0)
	rgbL += (rgbNW + rgbNE + rgbSW + rgbSE);
	rgbL *= FxaaToFloat3(1.0 / 9.0);
#endif
	float lumaNW = FxaaLuma(rgbNW);
	float lumaNE = FxaaLuma(rgbNE);
	float lumaSW = FxaaLuma(rgbSW);
	float lumaSE = FxaaLuma(rgbSE);
	float edgeVert =
		abs((0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE)) +
		abs((0.50 * lumaW) + (-1.0 * lumaM) + (0.50 * lumaE)) +
		abs((0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE));
	float edgeHorz =
		abs((0.25 * lumaNW) + (-0.5 * lumaW) + (0.25 * lumaSW)) +
		abs((0.50 * lumaN) + (-1.0 * lumaM) + (0.50 * lumaS)) +
		abs((0.25 * lumaNE) + (-0.5 * lumaE) + (0.25 * lumaSE));
	bool horzSpan = edgeHorz >= edgeVert;
#if FXAA_DEBUG_HORZVERT
	if (horzSpan) return FxaaFilterReturn(FxaaFloat3(1.0, 0.75, 0.0));
	else         return FxaaFilterReturn(FxaaFloat3(0.0, 0.50, 1.0));
#endif
	float lengthSign = horzSpan ? -rcpFrame.y : -rcpFrame.x;
	if (!horzSpan) lumaN = lumaW;
	if (!horzSpan) lumaS = lumaE;
	float gradientN = abs(lumaN - lumaM);
	float gradientS = abs(lumaS - lumaM);
	lumaN = (lumaN + lumaM) * 0.5;
	lumaS = (lumaS + lumaM) * 0.5;

	bool pairN = gradientN >= gradientS;
#if FXAA_DEBUG_PAIR
	if (pairN) return FxaaFilterReturn(FxaaFloat3(0.0, 0.0, 1.0));
	else      return FxaaFilterReturn(FxaaFloat3(0.0, 1.0, 0.0));
#endif
	if (!pairN) lumaN = lumaS;
	if (!pairN) gradientN = gradientS;
	if (!pairN) lengthSign *= -1.0;
	float2 posN;
	posN.x = pos.x + (horzSpan ? 0.0 : lengthSign * 0.5);
	posN.y = pos.y + (horzSpan ? lengthSign * 0.5 : 0.0);

	gradientN *= FXAA_SEARCH_THRESHOLD;
	float2 posP = posN;
	float2 offNP = horzSpan ?
		FxaaFloat2(rcpFrame.x, 0.0) :
		FxaaFloat2(0.0f, rcpFrame.y);
	float lumaEndN = lumaN;
	float lumaEndP = lumaN;
	bool doneN = false;
	bool doneP = false;
#if FXAA_SEARCH_ACCELERATION == 1
	posN += offNP * FxaaFloat2(-1.0, -1.0);
	posP += offNP * FxaaFloat2(1.0, 1.0);
#endif
#if FXAA_SEARCH_ACCELERATION == 2
	posN += offNP * FxaaFloat2(-1.5, -1.5);
	posP += offNP * FxaaFloat2(1.5, 1.5);
	offNP *= FxaaFloat2(2.0, 2.0);
#endif
#if FXAA_SEARCH_ACCELERATION == 3
	posN += offNP * FxaaFloat2(-2.0, -2.0);
	posP += offNP * FxaaFloat2(2.0, 2.0);
	offNP *= FxaaFloat2(3.0, 3.0);
#endif
#if FXAA_SEARCH_ACCELERATION == 4
	posN += offNP * FxaaFloat2(-2.5, -2.5);
	posP += offNP * FxaaFloat2(2.5, 2.5);
	offNP *= FxaaFloat2(4.0, 4.0);
#endif
	for (int i = 0; i < FXAA_SEARCH_STEPS; i++)
	{
#if FXAA_SEARCH_ACCELERATION == 1
		if (!doneN) lumaEndN =
			FxaaLuma(FxaaTexLod0(tex, posN.xy).xyz);
		if (!doneP) lumaEndP =
			FxaaLuma(FxaaTexLod0(tex, posP.xy).xyz);
#else
		if (!doneN) lumaEndN =
			FxaaLuma(FxaaTexGrad(tex, posN.xy, offNP).xyz);
		if (!doneP) lumaEndP =
			FxaaLuma(FxaaTexGrad(tex, posP.xy, offNP).xyz);
#endif
		doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
		doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);
		if (doneN && doneP) break;
		if (!doneN) posN -= offNP;
		if (!doneP) posP += offNP;
	}

	float dstN = horzSpan ? pos.x - posN.x : pos.y - posN.y;
	float dstP = horzSpan ? posP.x - pos.x : posP.y - pos.y;
	bool directionN = dstN < dstP;
#if FXAA_DEBUG_NEGPOS
	if (directionN) return FxaaFilterReturn(FxaaFloat3(1.0, 0.0, 0.0));
	else           return FxaaFilterReturn(FxaaFloat3(0.0, 0.0, 1.0));
#endif
	lumaEndN = directionN ? lumaEndN : lumaEndP;
	if (((lumaM - lumaN) < 0.0) == ((lumaEndN - lumaN) < 0.0))
		lengthSign = 0.0;

	float spanLength = (dstP + dstN);
	dstN = directionN ? dstN : dstP;
	float subPixelOffset = (0.5 + (dstN * (-1.0 / spanLength))) * lengthSign;
#if FXAA_DEBUG_OFFSET
	float ox = horzSpan ? 0.0 : subPixelOffset*2.0 / rcpFrame.x;
	float oy = horzSpan ? subPixelOffset*2.0 / rcpFrame.y : 0.0;
	if (ox < 0.0) return FxaaFilterReturn(
		FxaaLerp3(FxaaToFloat3(lumaO),
				  FxaaFloat3(1.0, 0.0, 0.0), -ox));
	if (ox > 0.0) return FxaaFilterReturn(
		FxaaLerp3(FxaaToFloat3(lumaO),
				  FxaaFloat3(0.0, 0.0, 1.0), ox));
	if (oy < 0.0) return FxaaFilterReturn(
		FxaaLerp3(FxaaToFloat3(lumaO),
				  FxaaFloat3(1.0, 0.6, 0.2), -oy));
	if (oy > 0.0) return FxaaFilterReturn(
		FxaaLerp3(FxaaToFloat3(lumaO),
				  FxaaFloat3(0.2, 0.6, 1.0), oy));
	return FxaaFilterReturn(FxaaFloat3(lumaO, lumaO, lumaO));
#endif
	float3 rgbF = FxaaTexLod0(tex, FxaaFloat2(
		pos.x + (horzSpan ? 0.0 : subPixelOffset),
		pos.y + (horzSpan ? subPixelOffset : 0.0))).xyz;
#if FXAA_SUBPIX == 0
	return FxaaFilterReturn(rgbF);
#else        
	return FxaaFilterReturn(FxaaLerp3(rgbL, rgbF, blendL));
#endif
}

float4 main(PixelShaderInput input) : SV_TARGET
{
	FxaaTex tex = {texSampler, mainTex};
	return float4(FxaaPixelShader(input.texcoord, tex, invScreenSize.xy), 1.0f);
}