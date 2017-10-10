#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>

#include <pthread.h>

#define LOGE(fmt, argv...) \
	printf(fmt, ##argv)
#define LOGD(fmt, argv...) \
	printf(fmt, ##argv)

static int FLAG_RUN = 1;
static int FLAG_LOAD = 0;

static int p1 = 0;

static void reload_param(int *p)
{
	*p = p1;
	FLAG_LOAD = 0;
}

static void * pthread_func(void* param)
{
	pthread_t thread = 0;
	thread = pthread_self();
	printf("thread id: %ld\n", thread);

	int i = 0;

	while(FLAG_RUN)
	{
		for(; i<20 && FLAG_RUN; i++)
		{
			if(FLAG_LOAD)
				reload_param(&i);
			printf("[%ld] %d\n", thread, i);
			sleep(1);
		}

	}

	printf("[%ld] exit",thread);
	return NULL;
}

static void
display_thread_attributes(pthread_t thread, char*prefix)
{
	int s;
	pthread_attr_t attr;
	if(pthread_getattr_np(thread, &attr) != 0)
	{
		perror("pthread_getattr_np");
		return;
	}

	size_t stack_size = 0;
	pthread_attr_getstacksize(&attr, &stack_size);
	printf("pthread:[%ld] stack size : %ld\n", thread, stack_size);
	
	
	if(pthread_attr_destroy(&attr) != 0)
	{
		perror("pthread_attr_destroy");
		return;
	}
}

int main(int argc, const char* argv[])
{
	display_thread_attributes(pthread_self(), NULL);

	pthread_t thread_id;
	pthread_attr_t attr;
	
	if(pthread_attr_init(&attr) != 0)
		perror("pthread_attr_init"),exit(-1);

/*
	if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
		perror("pthread_attr_setdetachstate"),exit(-1);
*/

	if(pthread_create(&thread_id, &attr, pthread_func, NULL) != 0)
		perror("pthread_create"),exit(-1);
	
	if(pthread_attr_destroy(&attr) != 0)
		perror("pthread_attr_destroy"),exit(-1);

	
	display_thread_attributes(thread_id, NULL);

	sleep(10);
	FLAG_LOAD=1;

	pthread_join(thread_id, NULL);


	exit(0);
		
}

