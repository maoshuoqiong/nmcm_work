#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/prctl.h>
#include <sys/capability.h>

#define TAG "HOOKTEST"

#define LOGD(fmt, args...) \
	__android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##args)

#define LOGE(fmt, args...) \
	__android_log_print(ANDROID_LOG_ERROR, TAG, fmt, ##args)

#define JAR_PATH "/data/data/com.example.maoshuoqiong.sendmessage/cache/handler.jar"
#define DEX_PATH "/data/data/com.example.maoshuoqiong.sendmessage/cache/handler.dex"

#define DEX2OAT_ARGS \
		"--instruction-set=arm64", \
		"--instruction-set-features=default", \
		"--runtime-arg", "-Xrelocate", \
		"--boot-image=/system/framework/boot.art", \
		"--dex-file="JAR_PATH, \
		"--oat-location="DEX_PATH, \
		"--runtime-arg", "-Xms64m", \
		"--runtime-arg","-Xmx512m", \
		NULL


#define BUFF_LEN 4096
#define PATH_LEN 1024
#define FILE_ACCESS ( S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH )
#define PATH_ACCESS ( S_IRWXU|S_IRWXG|S_IRWXO )

static const char* SRC_JAR_PATH = "/data/local/tmp/handler.jar";

static int open_oat(const char* oat)
{
	if(oat == NULL)
		return -1;

	int fd = -1;
	if(( fd = open(oat, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
	{
		LOGE("open %s fail: %s", oat, strerror(errno));
		return -1;
	}

	return fd;
}

static int switch_user()
{

	if(prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) != 0)
	{
		LOGE("prctl error: %s", strerror(errno));
		return -1;
	}

	struct passwd * pw = NULL;
	errno = 0;
	if(( pw = getpwnam("u0_a134")) == NULL)
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
	
	header.version = _LINUX_CAPABILITY_VERSION_3;
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

static int copy_file()
{
	/* copy handler.jar */
	if(access(JAR_PATH, F_OK) != 0)
	{
		LOGE("access error: %s", strerror(errno));
		if(copy(SRC_JAR_PATH, JAR_PATH) == 0)
		{
			/*chown_dest(JAR_PATH); */
			return 0;
		}
		else
			return -1;
	}

	return 0;

}

int main(int argc, const char* argv[])
{

	if(switch_user() != 0)
		return -1;

	if(copy_file() == -1)
		return -1;

	int oat = -1;
	if( (oat = open_oat(DEX_PATH)) == -1)
		return -1;
	
	pid_t pid = -1;
	if( (pid = fork()) == 0)
	{
		/* child */
		LOGD("Child : pid = %d, uid=%d, gid=%d, pgid=%d", getpid(), getuid(), getgid(), getpgrp());
		setpgid(0,0);
		LOGD("child: pid = %d, uid=%d, gid=%d, pgid=%d", getpid(), getuid(), getgid(), getpgrp());
		char sz[1024] = {0x00};
		sprintf(sz, "--oat-fd=%d", oat);
		execl("/system/bin/dex2oat","dex2oat", sz, DEX2OAT_ARGS);
		exit(-1);
	}
	else if( pid > 0)
	{
		close(oat);
		LOGD("Parent: pid = %d, uid=%d, gid=%d, pgid=%d", getpid(), getuid(), getgid(), getpgrp());
		int status = -1;
		LOGD("Fork success : %d", pid);
		if(waitpid(pid, &status , 0) != pid)
		{
			LOGE("waitpid error: %s", strerror(errno));
			return -1;
		}
		LOGD("status: %d", status);
		return 0;
	}
	else
	{
		LOGE("Fork error:%s", strerror(errno));
		return -1;
	}
	return 0;	
}

