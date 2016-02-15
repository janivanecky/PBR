#include "CHWin32.h"
#include "CHDef.h"

READFILE(Win32ReadFile)
{
	HANDLE fileHandle = 0;
	fileHandle = CreateFile2(fileName, GENERIC_READ, NULL, OPEN_EXISTING, NULL);
	FileData result = { 0 };
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		WIN32_FILE_ATTRIBUTE_DATA fileAttributes;
		if (GetFileAttributesExW(fileName, GetFileExInfoStandard, &fileAttributes))
		{
			UINT fileSize = fileAttributes.nFileSizeLow;
			HANDLE heap = GetProcessHeap();
			result.data = HeapAlloc(heap, HEAP_ZERO_MEMORY, fileSize);
			if (result.data)
			{
				DWORD bytesReadFromFile = 0;
				if (ReadFile(fileHandle, result.data, fileSize, &bytesReadFromFile, NULL) &&
					bytesReadFromFile == fileSize)
				{
					result.size = bytesReadFromFile;
				}
				else
				{
					HeapFree(heap, NULL, result.data);
					WIN_ERROR_DEBUG_FILE_OUTPUT_CALL("ReadFile", "Win32ReadFile", fileName);
				}
			}
			else
			{
				WIN_ERROR_DEBUG_FILE_OUTPUT_CALL("HeapAlloc", "Win32ReadFile", fileName);
			}
		}
		else
		{
			WIN_ERROR_DEBUG_FILE_OUTPUT_CALL("GetFileAttributesExW", "Win32ReadFile", fileName);
		}
		CloseHandle(fileHandle);
	}
	else
	{
		WIN_ERROR_DEBUG_FILE_OUTPUT_CALL("CreateFile2", "Win32ReadFile", fileName);
	}
	return result;
}

RELEASEFILE(Win32ReleaseFile)
{
	HANDLE heap = GetProcessHeap();
	HeapFree(heap, NULL, file.data);
}

WRITEFILE(Win32WriteFile)
{
	HANDLE fileHandle = 0;
	fileHandle = CreateFileW(fileName, GENERIC_WRITE, NULL, NULL, CREATE_NEW,
							 FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		DeleteFileW(fileName);
		fileHandle = CreateFileW(fileName, GENERIC_WRITE, NULL, NULL, CREATE_NEW,
								 FILE_ATTRIBUTE_NORMAL, NULL);
	}

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD writtenBytes;
		WriteFile(fileHandle, data, size, &writtenBytes, NULL);
		if (writtenBytes <= 0)
		{
			WIN_ERROR_DEBUG_FILE_OUTPUT_CALL("WriteFile", "Win32WriteFile", fileName);
		}
		CloseHandle(fileHandle);
	}
	else
	{
		WIN_ERROR_DEBUG_FILE_OUTPUT_CALL("CreateFileW", "Win32WriteFile", fileName);
	}

}