#pragma once
#include <windows.h>
#include "Core\Def.h"

struct FileData
{
	void *data;
	uint32 size;
};

#define READFILE(name) FileData name(wchar_t *fileName)
#define RELEASEFILE(name) void name(FileData file)
#define WRITEFILE(name) void name(wchar_t *fileName, void *data, unsigned int size)

typedef READFILE(PlatformReadFile);
typedef RELEASEFILE(PlatformReleaseFile);
typedef WRITEFILE(PlatformWriteFile);

READFILE(Win32ReadFile);
RELEASEFILE(Win32ReleaseFile);
WRITEFILE(Win32WriteFile);

