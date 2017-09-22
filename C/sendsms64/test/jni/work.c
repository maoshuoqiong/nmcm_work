#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <errno.h>
#include <stdint.h>
#include <android/log.h>
#include <jni.h>
#include <sys/mman.h>

#define  LOGI(fmt, args...) \
    __android_log_print( ANDROID_LOG_INFO, "HOOKTEST", fmt, ##args)

#define  LOGD(fmt, args...) \
    __android_log_print( ANDROID_LOG_DEBUG, "HOOKTEST", fmt, ##args)

#define  LOGE(fmt, args...) \
    __android_log_print( ANDROID_LOG_ERROR, "HOOKTEST", fmt, ##args)

#ifdef __cplusplus
extern "C" {
#endif


int main(void)
{
	LOGE("Begin[%d]....",getpid());

/*
	close(0);
	close(1);
	close(2);

	fopen("/dev/null","w+");
	fopen("/dev/null","w+");
	fopen("/dev/null","w+");
*/
	void *p = mmap(NULL, 0x4000, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if(p == MAP_FAILED)
		perror("mmap"),exit(-1);

	LOGE("mmap success :[%p]",p);
	munmap(p,0x4000);

	for(int i=0;i<10000;i++)
	{
		LOGE("%d",i);
		sleep(1);
	}

	return 0;
}

#ifdef __cplusplus
}
#endif
