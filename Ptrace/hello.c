#include <stdio.h>
#include <unistd.h>

#define LOG_TAG "DEBUG"
#define LOGD(fmt, args...) printf(fmt,##args)

int /* __attribute__((weak)) */ hook_entry(char* a)
{
	LOGD("Hook success, pid= %d\n",getpid());
	LOGD("Hello %s\n",a);
	LOGD("printf %p\n",printf);
	return 0;
}
