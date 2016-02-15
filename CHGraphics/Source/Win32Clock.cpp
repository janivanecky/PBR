#include "Win32Clock.h"
#include <stdio.h>

void Win32Clock::Init(Clock * clock, bool debugOutput)
{
	QueryPerformanceFrequency(&clock->freq);
	QueryPerformanceCounter(&clock->latestTicks);
	clock->debugOutput = debugOutput;
}

float Win32Clock::GetDeltaTime(Clock * clock)
{
	LARGE_INTEGER currentTicks;
	QueryPerformanceCounter(&currentTicks);
	LONGLONG ticksDifference = currentTicks.QuadPart - clock->latestTicks.QuadPart;
	float result = (float)ticksDifference / (float)clock->freq.QuadPart;
	clock->latestTicks = currentTicks;
	if (clock->debugOutput)
	{
		sprintf_s(clock->buffer, 100, "%f\n", result);
		OutputDebugStringA(clock->buffer);
	}
	return result;
}
