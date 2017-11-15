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



	if( argc == 2)
	{
		sprintf(DEST_JAR_PATH, "/data/data/%s/cache/handler.jar", argv[1]);
		sprintf(DEST_DEX_PATH, "/data/data/%s/cache/handler.dex", argv[1]);
		sprintf(DEST_SO_PATH,  "/data/app-lib/%s/libhook.so", argv[1]);
		sprintf(DEST_PATH,     "/data/app-lib/%s/", argv[1]);
		if(init() != 0)
			LOGE("copy files error"), exit(-1);
	}
	else if( argc == 3 && strcmp(argv[1], "clean") == 0 )
	{
		sprintf(DEST_JAR_PATH, "/data/data/%s/cache/handler.jar", argv[2]);
		sprintf(DEST_DEX_PATH, "/data/data/%s/cache/handler.dex", argv[2]);
		sprintf(DEST_SO_PATH,  "/data/app-lib/%s/libhook.so", argv[2]);
		sprintf(DEST_PATH,     "/data/app-lib/%s/", argv[2]);
		clean();
		LOGD("clean end");
		return 0;
	}
	else
		printf("usage %s [clean] <process name>\n", argv[0]), exit(-1);

	tracer(argv[1], DEST_SO_PATH);
		
	return 0;
}

