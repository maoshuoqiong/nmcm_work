
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/file.h>
#include <assert.h>
#include <signal.h>

#include <pwd.h>

#include "hook.h"
#include "log.h"

#define BUFF_LEN 4096
#define PATH_LEN 1024
#define FILE_ACCESS ( S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH )
#define PATH_ACCESS ( S_IRWXU|S_IRWXG|S_IRWXO )
#define RADIO_NAME "radio"

static const char* DEST_PATH  = "/data/app-lib/com.android.phone/";
static const char* SRC_JAR_PATH   = "/data/local/tmp/handler.jar";
static const char* DEST_JAR_PATH  = "/data/data/com.android.phone/cache/handler.jar";

static const char* SRC_SO_PATH   = "/data/local/tmp/libsm.so";
static const char* DEST_SO_PATH  = "/data/app-lib/com.android.phone/libsm.so";
/*
static const char* DEST_SO_PATH  = "/data/data/com.android.phone/cache/libsm.so";
*/

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
	if( (pw = getpwnam(RADIO_NAME)) == NULL )
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

static int clean()
{
	if(unlink(DEST_JAR_PATH) != 0)
		LOGE("delete %s error: %s", DEST_JAR_PATH, strerror(errno));

	if(unlink(DEST_SO_PATH) != 0)
		LOGE("delete %s error: %s", DEST_SO_PATH, strerror(errno));

	return 0;
}

int main(int arcv,char * argv[]){

	if(arcv == 2 && strcmp(argv[1], "clean") == 0 )
	{
		clean();
		LOGD("clean end");
		return 0;
	}

	if(init() != 0)
		LOGE("copy files wrong"),exit(-1);

    LOGE("[+] arcv:%d ", arcv);
    LOGE("[+] argv:%s ", argv[0]);
    LOGE("[+] argv:%s ", argv[1]);
    LOGE("[+] argv:%s ", argv[2]);
    LOGE("[+] argv:%s ", argv[3]);

    if((argv[1] != NULL)&&(argv[2] != NULL)&&(argv[3] != NULL))
    {
        LOGE("[+] arcv:test");
        if ((*argv[3])=='1'){
            LOGE("[+] arcv:test3 ");
            hook_send_text_sms(argv[1],argv[2]);
        }else{
            LOGE("[+] arcv:test4 ");
            hook_send_data_sms(argv[1],argv[2],0);
        }
    }

    
}



