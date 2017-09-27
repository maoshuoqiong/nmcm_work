#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>

#define LOGE(fmt, argv...) \
	printf(fmt, ##argv)
#define LOGD(fmt, argv...) \
	printf(fmt, ##argv)

#define SOCK_PATH "/dev/socket/"

int main(int argc, const char* argv[])
{
	if(argc < 2)
		LOGE("Usage %s <localsocket>\n", argv[0]),exit(-1);

	char szName[1024] = SOCK_PATH;
	strcat(szName, argv[1]);
	size_t namelen = strlen(szName);
	LOGD("socket file:[%s], len[%d]\n", szName, namelen);
	
	int sd;
	if( (sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		perror("socket error"),exit(-1);
	LOGD("socket success\n");

	struct sockaddr_un addr;
	socklen_t alen;
	int err;

	alen = namelen + offsetof(struct sockaddr_un, sun_path) + 1;
	
	bzero(&addr, sizeof addr);
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, szName);

	if(connect(sd, (struct sockaddr *)&addr, alen) < 0)
	{
		perror("connect error");
		close(sd);
		exit(-1);
	}

	LOGD("connect success\n");
	char buf[4096] = {0x00};
	int nread = 0;
	nread = read( sd, buf, sizeof(buf));
	LOGD("read len[%d]: %s\n", nread, buf );
	
	close(sd);
	return 0;
}

