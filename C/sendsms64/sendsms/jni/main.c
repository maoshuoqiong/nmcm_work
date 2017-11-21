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
#include <sys/prctl.h>
#include <sys/capability.h>
#include <pwd.h>
#include <sys/wait.h>

#include "hook.h"
#include "log.h"

#define BUFF_LEN 4096
#define PATH_LEN 1024
#define FILE_ACCESS ( S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH )
#define PATH_ACCESS ( S_IRWXU|S_IRWXG|S_IRWXO )
#define RADIO_NAME "radio"

#define DEST_JAR_PATH "/data/data/com.android.phone/cache/handler.jar"
#define DEST_DEX_PATH "/data/data/com.android.phone/cache/handler.dex"

#if defined(__aarch64__)
#define ARCH "arm64"
#elif defined(__arm__)
#define ARCH "arm"
#else
#error NOT Support
#endif

#define DEX2OAT_ARGS \
		"--instruction-set="ARCH, \
		"--instruction-set-features=default", \
		"--runtime-arg", "-Xrelocate", \
		"--boot-image=/system/framework/boot.art", \
		"--dex-file="DEST_JAR_PATH, \
		"--oat-location="DEST_DEX_PATH, \
		"--runtime-arg", "-Xms64m", \
		"--runtime-arg","-Xmx512m", \
		NULL

static const char* DEST_PATH  = "/data/app-lib/com.android.phone/";
static const char* SRC_JAR_PATH   = "/data/local/tmp/handler.jar";
static const char* SRC_SO_PATH   = "/data/local/tmp/libsm.so";
static const char* DEST_SO_PATH  = "/data/app-lib/com.android.phone/libsm.so";

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
	{
		if(( nwrite = write(dest_fd, buff, nread)) < nread)
		{
			LOGE("write count : %d, read %d", nwrite, nread);
			if(nread == -1)
				LOGE("error: %s", strerror(errno));

			ret = -1;
			goto exit1; 
		}
	}
	
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

static int switch_user()
{
	if(prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) != 0)
	{
		LOGE("prctl err: %s", strerror(errno));
		return -1;
	}
	
	struct passwd * pw = NULL;
	errno = 0;
	if( (pw = getpwnam(RADIO_NAME)) == NULL )
	{
		LOGE("getpwnam error: %s", strerror(errno));
		return -1;
	}

	if(setuid(pw->pw_uid) != 0)
	{
		LOGE("setuid error: %s", strerror(errno));
		return -1;
	}

	struct __user_cap_header_struct header;
	struct __user_cap_data_struct data;
	
	header.version = _LINUX_CAPABILITY_VERSION;
	header.pid = getpid();

	if(capget(&header, &data) != 0)
	{
		LOGE("capget error: %s", strerror(errno));
		return -1;
	}

	LOGD("[capability]: effective=%u, permitted=%u, inheritable=%u", data.effective, data.permitted, data.inheritable);

	data.effective = data.inheritable = data.permitted;

	if(capset(&header, &data) != 0)
	{
		LOGE("capset error: %s", strerror(errno));
		return -1;
	}

	if(setgid(pw->pw_gid) != 0)
	{
		LOGE("setgid error: %s", strerror(errno));
		return -1;
	}

	return 0;
}

static int copy_files()
{
	/* umask(0); */

	/* makedir */	
	if(access(DEST_PATH, F_OK) != 0)
	{
		LOGE("access PATH error: %s", strerror(errno));
		if(mkdir(DEST_PATH, PATH_ACCESS) != 0)
		{
			LOGE("mkdir error: %s", strerror(errno));
			return -1;
		}
	}	
	
	/* copy handler.jar */
	if(access(DEST_JAR_PATH, F_OK) != 0)
	{
		LOGE("access JAR error: %s", strerror(errno));
		if(copy(SRC_JAR_PATH, DEST_JAR_PATH) != 0)
			return -1;
	}

	/* copy libsm.so */
	if(access(DEST_SO_PATH, F_OK) != 0)
	{
		LOGE("access SO error: %s", strerror(errno));
		if(copy(SRC_SO_PATH, DEST_SO_PATH) != 0)
			return -1;
	}

	return 0;
}

static int init()
{

	/* switch user */
	if(switch_user() != 0)
		return -1;

	/* copy files */
	if(copy_files() != 0)
		return -1;

	return 0;

	/* dex2oat */
	if(access(DEST_DEX_PATH, F_OK) == 0) /* .dex file already exist */
		return 0;

	int oat_fd = -1;
	if(( oat_fd = open(DEST_DEX_PATH, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR )) < 0)
	{
		LOGE("open %s error: %s", DEST_DEX_PATH, strerror(errno));
		return -1;
	}

	pid_t pid = -1;
	if( (pid = fork()) == -1)
	{
		LOGE("fork error: %s", strerror(errno));
		return -1;
	}
	else if( pid == 0)
	{
		/* child */
		setpgid(0,0);
		
		char sz[1024] = {0x00};
		sprintf(sz, "--oat-fd=%d", oat_fd);
		execl("/system/bin/dex2oat", "dex2oat", sz, DEX2OAT_ARGS);
		LOGE("execl error");
		exit(-1);
	}
	else
	{
		close(oat_fd);
		/* parent */
		LOGD("Fork success: %d", pid);
		
		int status = -1;
		if(waitpid(pid, &status, 0) != pid)
		{
			LOGE("waitpid error: %s", strerror(errno));
			return -1;
		}

		LOGD("status: %d", status);
		if(!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		{
			LOGE("Failed execv child");
			return -1;
		}
		return 0;
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

int main(int arcv,char * argv[]){

	if(arcv == 2 && strcmp(argv[1], "clean") == 0 )
	{
		clean();
		LOGD("clean end");
		return 0;
	}

	if(init() != 0)
		LOGE("init wrong"),exit(-1);

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



