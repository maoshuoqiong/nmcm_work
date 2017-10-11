#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include <dlfcn.h>
#include <fcntl.h>

#define LIBPATH "/system/lib64/mtk-ril.so"
#define AT_PATH "/dev/pts/5"


#define LOGE(fmt, argv...) \
	printf(fmt, ##argv)
#define LOGD(fmt, argv...) \
	printf(fmt, ##argv)

typedef void (*ATUnsolHandler)(const char*s, const char *sms_pdu);

typedef int AT_open(int fd, ATUnsolHandler h);
typedef void AT_close();

static void onUnsoled(const char* s, const char *sms_pdu)
{
	/*
	printf("onUnsoled:[%s], [%s]\n", s, sms_pdu);
	*/
	printf("onUnsoled");
}

int main(int argc, const char* argv[])
{
	int fid = -1;

	if( (fid = open(AT_PATH, O_RDWR))<0 )
		perror("open"),exit(-1);
	
	void *handle = NULL;
	if( (handle = dlopen(LIBPATH, RTLD_NOW)) ==NULL)
		LOGE("dlopen error: %s\n",dlerror()),exit(-1);

	AT_open *at_open= NULL;
	if( (at_open = (AT_open*)dlsym(handle, "at_open")) == NULL)
		LOGE("dlsym error: %s\n", dlerror());	

	AT_close *at_close = NULL;
	if( (at_close = (AT_close*)dlsym(handle, "at_close")) == NULL)
		LOGE("dlsym error: %s\n", dlerror());	

	if(at_open(fid, onUnsoled)<0)
		LOGE("AT error on at_open\n");

	getchar();

	at_close();
	
	if(dlclose(handle) != 0)
		LOGE("dlclose error: %s\n",dlerror()),exit(-1);	

	return 0;	
}

