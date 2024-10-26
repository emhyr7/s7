#include <Windows.h>

#include <assert.h>

typedef __UINT32_TYPE__ UINT32;
typedef __UINT64_TYPE__ UINT64;

typedef char ASCII;

HANDLE open_file(const ASCII *path)
{
	HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	assert(file);
	return file;
}

UINT64 get_size_of_file(HANDLE file)
{
	LARGE_INTEGER file_size;
	assert(GetFileSizeEx(file, &file_size));
	return file_size.QuadPart;
}

UINT32 read_from_file(void *buffer, UINT32 file_size, HANDLE file)
{
	DWORD bytes_read_size;
	assert(ReadFile(file, buffer, file_size, &bytes_read_size, 0));
	return bytes_read_size;
}

void *allocate(UINT32 size)
{
	void *memory = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	assert(memory);
	return memory;
}
