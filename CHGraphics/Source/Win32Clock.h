#pragma once
#include <windows.h>

struct Clock
{
	LARGE_INTEGER freq;
	LARGE_INTEGER latestTicks;
	bool debugOutput;
	char buffer[100];
};

namespace Win32Clock
{
	void Init(Clock *clock, bool debugOutput = false);
	float GetDeltaTime(Clock *clock);
}