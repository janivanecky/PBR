#pragma once
#include <cinttypes>
#define WIN_ERROR_DEBUG_OUTPUT(a) OutputDebugStringA(a"\n")
#define WIN_ERROR_DEBUG_OUTPUT_CALL(a,b) OutputDebugStringA("Error calling "a"() at "b"()\n")
#define WIN_ERROR_DEBUG_FILE_OUTPUT_CALL(a,b,c) OutputDebugStringA("Error calling "a"() at "b"(); working with file: "); OutputDebugStringW(c); OutputDebugStringA("\n");
#define WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(x,y,z) if(FAILED(x)) {WIN_ERROR_DEBUG_OUTPUT_CALL(y,z);}
#define RELEASE_DX_RESOURCE(x) if(x) x->Release(); x = NULL;

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef int32_t int32;

struct Color
{
	union
	{
		float x[4];
		struct
		{
			float r;
			float g;
			float b;
			float a;
		};
	};

	Color()
	{
		this->r = 0;
		this->g = 0;
		this->b = 0;
		this->a = 0;
	}

	Color(float r, float g, float b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = 1;
	}

	Color(float r, float g, float b, float a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	Color(uint32 hex)
	{
		this->r = (float)((hex & 0xff0000) >> 16) / 255.0f;
		this->g = (float)((hex & 0x00ff00) >> 8) / 255.0f;
		this->b = (float)(hex & 0x0000ff) / 255.0f;
		this->a = 1;
	}

	Color operator*(float x)
	{
		return Color(this->r * x, this->g * x, this->b * x, this->a * x);
	}

	bool operator==(Color a)
	{
		return this->r == a.r && this->g == a.g && this->b == a.b && this->a == a.a;
	}

};