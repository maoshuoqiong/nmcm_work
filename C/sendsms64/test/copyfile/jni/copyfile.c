#include <stdio.h>
#include <jni.h>
#include <android/log.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>

static const char* TAG = "COPYFILE";
static const char* TMP_PATH= "/data/local/tmp/";
static const char* CACHE_PATH = "/data/data/com.android.phone/cache/";

#define LOGD(fmt, args...) \
	__android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##args)

#define LOGE(fmt, args...) \
	__android_log_print(ANDROID_LOG_ERROR, TAG, fmt, ##args)

#define BUFF_LEN 4096
#define PATH_LEN 1024
#define FILE_ACCESS ( S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH )
#define PATH_ACCESS ( S_IRWXU|S_IRWXG|S_IRWXO )
#define RADIO_NAME "radio"

static int copy(const char* src, const char* dest)
{
	if(src == NULL || dest == NULL)
		return -1;

	int src_fd, dest_fd;
	int ret = 0;

	if( (src_fd = open(src, O_RDONLY)) <0 )
	{
		LOGE("open file [%s] error: %s", src, strerror(errno));
		ret = -1;
		goto exit3;
	}

	if( (dest_fd = open(dest, O_WRONLY|O_CREAT|O_TRUNC, FILE_ACCESS)) <0 )
	{
		LOGE("open file [%s] error: %s", dest, strerror(errno));
		ret = -1;
		goto exit2;
	}

	char buff[BUFF_LEN] = {0x00};
	int nread =0, nwrite = 0;
	
	while( (nread = read(src_fd, buff, BUFF_LEN)) >0)
		write(dest_fd, buff, nread);

	if(nread < 0)
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

	struct passwd *pw = NULL;
	
	errno = 0;
	if( (pw = getpwnam(RADIO_NAME)) == NULL)
	{
		LOGE("getpwnam error: %s", strerror(errno));
		return -1;
	}

	LOGD("[%s]: uid[%d], gid[%d], dir[%s], name[%s], passwd[%s], shell[%s]",
         RADIO_NAME, pw->pw_uid, pw->pw_gid, pw->pw_dir,
         pw->pw_name, pw->pw_passwd, pw->pw_shell);

	if(chown(dest, pw->pw_uid, pw->pw_gid)<0)
	{
		LOGE("chown error: %s", strerror(errno));
		return -1;
	}
	
	return 0;
}

int main(int argc, const char * argv[])
{
	if(argc < 2 || argc > 3)
		printf("usage: %s <file> {dest}\n"
			"Notes:<file> must in path /data/local/tmp/\n", argv[0]),exit(-1);

	char src[PATH_LEN] = {0x00};
	char dest[PATH_LEN] = {0x00};

	if(strlen(argv[1])+1 > PATH_LEN)
		printf("strings length can't larger than %d\n", PATH_LEN),exit(-1);

	strcpy(src, TMP_PATH);
	strcat(src, argv[1]);

	if(argc == 3)
	{
		int len = strlen(argv[2]);
		if(len+1 > PATH_LEN)
			printf("strings length can't larger than %d\n", PATH_LEN),exit(-1);
			
		strcpy(dest, argv[2]);
		if(dest[len] != '/')
			strcat(dest,"/");

		if(access(dest, F_OK) != 0)
		{
			LOGE("access error: %s", strerror(errno));
			if(mkdir(dest, PATH_ACCESS) != 0)
				LOGE("mkdir error: %s", strerror(errno)),exit(-1);
		}
	}
	else
		strcpy(dest, CACHE_PATH);

	strcat(dest, argv[1]);
 	
	LOGD("src=[%s], dest=[%s]", src, dest);

	if(copy(src, dest) == 0)
		chown_dest(dest);
	
	return 0;
}

