#include <jni.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>

#include "log.h"
#include "tracer.h"

#define BUFF_LEN 4096
#define PATH_LEN 1024
#define FILE_ACCESS ( S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH )
#define PATH_ACCESS ( S_IRWXU|S_IRWXG|S_IRWXO )
#define USER_NAME "u0_a134"

#define MAX_PATH 1024
static char DEST_PATH[MAX_PATH];
static char DEST_JAR_PATH[MAX_PATH];
static char DEST_DEX_PATH[MAX_PATH];
static char DEST_SO_PATH[MAX_PATH];

static const char* SRC_JAR_PATH = "/data/local/tmp/handler.jar";
static const char* SRC_DEX_PATH = "/data/local/tmp/handler.dex";
static const char* SRC_SO_PATH  = "/data/local/tmp/libhook.so";

static int copy(const char*src, const char* dest)
{
	if(src == NULL || dest == NULL)
		return -1;
	
	int src_fd, dest_fd;
	int ret = 0;
	
	if( (src_fd = open(src, O_RDONLY)) <0)
	{
		LOGE("open file [%s] error: %s", src, strerror(errno));
		ret = -1;
		goto exit3;
	}
	
	if( (dest_fd = open(dest, O_WRONLY|O_CREAT|O_TRUNC, FILE_ACCESS)) <0)
	{
		LOGE("open file [%s] error: %s", dest, strerror(errno));
		ret = -1;
		goto exit2;
	}

	char buff[BUFF_LEN] = {0x00};
	int nread = 0, nwrite = 0;

	while(( nread = read(src_fd, buff, BUFF_LEN)) > 0 )
		write(dest_fd, buff, nread);
	
	if(nread <0)
	{
		LOGE("read file error: %s", strerror(errno));
		ret = -1;
	}

exit1:
	close(dest_fd);
exit2:
	close(src_fd);
exit3:
	return ret;
}

static int chown_dest(const char* dest)
{
	if(dest == NULL)
		return -1;

	struct passwd * pw = NULL;
	
	errno = 0;
	if( (pw = getpwnam(USER_NAME)) == NULL )
	{
		LOGE("getpwnam error: %s", strerror(errno));
		return -1;
	}

	if(chown(dest, pw->pw_uid, pw->pw_gid)<0)
	{
		LOGE("chown error: %s", strerror(errno));
		return -1;
	}

	return 0;
}

static int clean()
{
	if(unlink(DEST_JAR_PATH) != 0)
		LOGE("delete %s error: %s", DEST_JAR_PATH, strerror(errno));
	
	if(unlink(DEST_DEX_PATH) != 0)
		LOGE("delete %s error: %s", DEST_DEX_PATH, strerror(errno));

	if(unlink(DEST_SO_PATH) != 0)
		LOGE("delete %s error: %s", DEST_SO_PATH, strerror(errno));

	return 0;
}

static int init()
{
	umask(0);
	/* makedir */	
	if(access(DEST_PATH, F_OK) != 0)
	{
		LOGE("access error: %s", strerror(errno));
		if(mkdir(DEST_PATH, PATH_ACCESS) != 0)
		{
			LOGE("mkdir error: %s", strerror(errno));
			return -1;
		}
	}	
	
	/* copy handler.jar */
	if(access(DEST_JAR_PATH, F_OK) != 0)
	{
		LOGE("access error: %s", strerror(errno));
		if(copy(SRC_JAR_PATH, DEST_JAR_PATH) == 0)
			chown_dest(DEST_JAR_PATH);
		else
			return -1;
	}


	/* copy handler.dex*/
/*
	if(access(DEST_DEX_PATH, F_OK) != 0)
	{
		LOGE("access error: %s", strerror(errno));
		if(copy(SRC_DEX_PATH, DEST_DEX_PATH) == 0)
			chown_dest(DEST_DEX_PATH);
		else
			return -1;
	}
*/

	/* copy libsm.so */
	if(access(DEST_SO_PATH, F_OK) != 0)
	{
		LOGE("access error: %s", strerror(errno));
		if(copy(SRC_SO_PATH, DEST_SO_PATH) == 0)
			chown_dest(DEST_SO_PATH);
		else
			return -1;
	}

	return 0;
		
}

int main(int argc, const char* argv[])
{
	if(argc < 2 || argc >3)
		printf("usage %s [clean] <process name>\n", argv[0]), exit(-1);


	const char * tmp = NULL;

	if( argc == 2)
	{
	/*
		tmp = argv[1];
	*/
		tmp = "com.example.maoshuoqiong.sendmessage";
		sprintf(DEST_JAR_PATH, "/data/data/%s/cache/handler.jar", tmp);
		sprintf(DEST_DEX_PATH, "/data/data/%s/cache/handler.dex", tmp);
		sprintf(DEST_SO_PATH,  "/data/app-lib/%s/libhook.so", tmp);
		sprintf(DEST_PATH,     "/data/app-lib/%s/", tmp);
		if(init() != 0)
			LOGE("copy files error"), exit(-1);
		tracer(tmp, DEST_SO_PATH);
	}
	else if( argc == 3 && strcmp(argv[1], "clean") == 0 )
	{
	/*
		tmp = argv[2];
	*/
		tmp = "com.example.maoshuoqiong.sendmessage";
		sprintf(DEST_JAR_PATH, "/data/data/%s/cache/handler.jar", tmp);
		sprintf(DEST_DEX_PATH, "/data/data/%s/cache/handler.dex", tmp);
		sprintf(DEST_SO_PATH,  "/data/app-lib/%s/libhook.so", tmp);
		sprintf(DEST_PATH,     "/data/app-lib/%s/", tmp);
		clean();
		LOGD("clean end");
		return 0;
	}
	else
		printf("usage %s [clean] <process name>\n", argv[0]), exit(-1);

		
	return 0;
}

