#include <stdio.h>
#include <wait.h>
#include <stdint.h>

int main(int argc, const char* argv[])
{
	int a = 0x47f;
	if(__WIFSIGNALED(a))
	{
		printf("signaled\n");
		printf("%d\n", ((signed char)((a & 0x7f)+1) >> 1));
		printf("termsig :%d\n", __WTERMSIG(a));
	}

	if(__WIFSTOPPED(a))
	{
		printf("stopsig :%d\n", __WSTOPSIG(a));
	}

	
	return 0;
}

