#include <Windows.h>

#include <assert.h>
#include <stdio.h>

#undef stdin
#undef stdout
#undef stderr

typedef __INT32_TYPE__ SINT32;

typedef __UINT32_TYPE__ UINT32;
typedef __UINT64_TYPE__ UINT64;

typedef char ASCII;

HANDLE stdin;
HANDLE stdout;
HANDLE stderr;

HANDLE open_file(const ASCII *path)
{
	HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(file == INVALID_HANDLE_VALUE)
	{
		printf("Last error: %lu\n", GetLastError());
		DebugBreak();
	}
	return file;
}

UINT64 get_size_of_file(HANDLE file)
{
	LARGE_INTEGER file_size;
	BOOL result = GetFileSizeEx(file, &file_size);
	if(!result)
	{
		printf("Last error: %lu\n", GetLastError());
		DebugBreak();
	}
	return file_size.QuadPart;
}

UINT32 read_from_file(void *buffer, UINT32 size, HANDLE file)
{
	DWORD read_size;
	BOOL result = ReadFile(file, buffer, size, &read_size, 0);
	if(!result)
	{
		printf("Last error: %lu\n", GetLastError());
		DebugBreak();
	}
	return read_size;
}

UINT32 write_into_file(const void *buffer, UINT32 size, HANDLE file)
{
	DWORD written_size;
	BOOL result = WriteFile(file, buffer, size, &written_size, 0);
	if(!result)
	{
		printf("Last error: %lu\n", GetLastError());
		DebugBreak();
	}
	return written_size;
}

void *allocate(UINT32 size)
{
	void *memory = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if(!memory)
	{
		printf("Last error: %lu\n", GetLastError());
		DebugBreak();
	}
	return memory;
}

extern int start(int, char*[]);

int main(int argc, char *argv[])
{
	STARTUPINFOA startup_info;
	GetStartupInfoA(&startup_info);
	stdin  = GetStdHandle(STD_INPUT_HANDLE);
	stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	stderr = GetStdHandle(STD_ERROR_HANDLE);
	return start(argc, argv);
}
