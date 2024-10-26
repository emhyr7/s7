#include <Windows.h>
#include <stdio.h>

#undef stdin
#undef stdout
#undef stderr


int main(void)
{
	HANDLE stdin  = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE stderr = GetStdHandle(STD_ERROR_HANDLE);

	printf("%u %u %u", stdin, stdout, stderr);
	return 0;
}
