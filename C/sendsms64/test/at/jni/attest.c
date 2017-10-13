#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <pthread.h>

#define LIBPATH "/system/lib64/mtk-ril.so"
#define AT_PATH "/dev/pts/4"


#define LOGE(fmt, argv...) \
	printf(fmt, ##argv)
#define LOGD(fmt, argv...) \
	printf(fmt, ##argv)


#define MAX_AT_RESPONSE (8 * 1024)

typedef void (*ATUnsolHandler)(const char*s, const char *sms_pdu);

typedef int AT_open(int fd, ATUnsolHandler h);
typedef void AT_close();

static int s_id = -1;
static const char* s_pdu = NULL;

static void onUnsoled(const char* s, const char *sms_pdu)
{
	/*
	printf("onUnsoled:[%s], [%s]\n", s, sms_pdu);
	*/
	printf("onUnsoled");
}

static void usage(const char* arg)
{
	printf("usage: %s <phone> <message>\n", arg);
	exit(0);
}

static void write_ctrlz(const char* cmd)
{
	ssize_t written;
	ssize_t cur = 0;
	ssize_t len = strlen(cmd);

	LOGD("AT> %s(%ld)\n", cmd, len);
	while(cur < len)
	{
		do{
			written = write(s_id, cmd+cur, len - cur);
		}while(written<0 && errno == EINTR);

		if(written <0)
		{
			perror("write");
			return;
		}
		cur += written;
	}

	do{
		written = write(s_id, "\032", 1);
	}while( (written <0 && errno == EINTR) || (written ==0));

	if(written <0)
	{
		perror("write");
		return;
	}

}

static char* readline()
{
	ssize_t count;
	
	char p_buf[MAX_AT_RESPONSE+1] = {0x00};
	char *p_eol  = NULL;
	char *ret;
	char * p_read = p_buf;

	LOGD("reading ... \n");

	if( (count = read(s_id, p_buf, MAX_AT_RESPONSE)) < 0)
	{
		if(count == 0)
			LOGD("atchannel: EOF reached");
		else
			perror("read");
		return NULL;
	}
	p_buf[count] = '\0';
	LOGD("Read :[%s]\n",p_buf);
	
	while(*p_read == '\r' || *p_read == '\n')
		p_read++;

	if(s_pdu != NULL && 0== strcmp(p_read, "> "))
	{
		LOGD("write sms\n");
		write_ctrlz(s_pdu);
		s_pdu = NULL;
	}
	
	return NULL;
}

static void* readLoop(void* param)
{
	for(;;)
	{
	/*
		const char* line;
		line = readline();
		if(line == NULL)
			break;
		LOGD("%s\n", line);
		free(line);
	*/
		
		readline();
	}
	return NULL;
}

static void write_line(const char* cmd)
{
	ssize_t written;
	ssize_t cur = 0;
	ssize_t len = strlen(cmd);

	LOGD("AT> %s(%ld)\n", cmd, len);
	while(cur < len)
	{
		do{
			written = write(s_id, cmd+cur, len - cur);
		}while(written<0 && errno == EINTR);

		if(written <0)
		{
			perror("write");
			return;
		}
		cur += written;
	}

	do{
		written = write(s_id, "\r", 1);
	}while( (written <0 && errno == EINTR) || (written ==0));

	if(written <0)
	{
		perror("write");
		return;
	}
}


static void send_sms_msg(const char* num, const char* pdu)
{
/*
	write_line("AT+CMEE=1");
*/
	/* s_pdu = pdu; */
	/* s_pdu = "0891683110808805F0" */
#if 1
	write_line("AT+CMGS=19");
	s_pdu = "00"
			"2100"
			"0b815110040179f3"
			"0008"
			"064f60597d554a";
#else
	write_line("AT+CMGS=17");
	s_pdu = "00"
			"1100"
			"0D91685110040179F3"
			"000000"
			"02C834";
#endif
}

int main(int argc, const char* argv[])
{
	int fid = -1;

	if( (fid = open(AT_PATH, O_RDWR))<0 )
		perror("open"),exit(-1);

	s_id = fid;
	LOGD("open %s success, fd=[%d]\n", AT_PATH, fid);
/*	
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
*/
	
	if(argc <2)
		usage(argv[0]);

	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(pthread_create(&tid, &attr, readLoop, NULL) != 0)
	{
		perror("pthread_create");
		goto exit;
	}

	LOGD("Thread create success\n");
/*
	write_line("AT+CSCA?");
*/
	getchar();
	
/*
	write_line(argv[1]);
*/
	send_sms_msg(argv[1], argv[2]);
	getchar();

exit:
	pthread_attr_destroy(&attr);		
	close(fid);
	return 0;	
}

