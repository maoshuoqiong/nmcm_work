#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <wchar.h>

int main(int argc, const char * argv[])
{
	char buf[256] = "你好啊";
	printf("%lu\n", strlen(buf));

	for(int i=0;i<strlen(buf);i++)
		printf("%X ", buf[i]);
	printf("\n");

	char buf2[256] = "hello";
	printf("%lu\n", strlen(buf2));
	
	for(int i=0;i<strlen(buf2);i++)
		printf("%X ", buf2[i]);
	printf("\n");

	wchar_t buf3[256] = L"hello";
	printf("%lu\n", wcslen(buf3));

	for(int i=0;i<wcslen(buf3);i++)
		printf("%X ", buf3[i]);
	printf("\n");

	wchar_t buf4[256] = L"你好啊";
	printf("%lu\n", wcslen(buf4));

	for(int i=0;i<wcslen(buf4);i++)
		printf("%X ", buf4[i]);
	printf("\n");

	char * p = setlocale(LC_ALL,"");	
/*
	if(p == NULL)
		printf("setlocale error\n"),exit(-1);
	printf("%s\n",p);
*/

	wchar_t ar[256] = {L'\0'};
	int read = mbstowcs(ar, &buf, 9 );
	if(read < 0 )
		perror("mbstowcs error");
	printf("%d %lu\n", read, sizeof(wchar_t));

	printf("%ls\n", ar);

	for(int i=0;i<read;i++)
		printf("%X ", ar[i]);
	printf("\n");


	return 0;
}

