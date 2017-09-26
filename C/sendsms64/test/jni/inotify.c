#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <sys/inotify.h>

static void handle_events(int fd, int wd, int argc, const char * argv[])
{
	ssize_t len;
	char buf[4096];
	char *ptr;
	const struct inotify_event * event;

	if( (len = read(fd, buf, sizeof buf)) == -1 )
		if( errno != EAGAIN)
			perror("read"), exit(-1);

	printf("read len :%ld\n", len);

	for(ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len)
	{
		event = (const struct inotify_event*) ptr;
		printf("mask %08x, IN_ISDIR[%08x]\n", event->mask, IN_ISDIR);

		if(event->mask & IN_OPEN)
			printf("IN_OPEN[%08x] ",IN_OPEN);
		if(event->mask & IN_CLOSE_NOWRITE)
			printf("IN_CLOSE_NOWRITE[%08x] ", IN_CLOSE_NOWRITE);
		if(event->mask & IN_CLOSE_WRITE)
			printf("IN_CLOSE_WRITE[%08x] ", IN_CLOSE_WRITE);
		if(event->mask & IN_MODIFY)
			printf("IN_MODIFY[%08x] ", IN_MODIFY);

		if(event->len)
			printf("%s", event->name);
		
		if(event->mask & IN_ISDIR)
			printf(" [directory]\n");
		else
			printf(" [file]\n");
	}
		
	printf("handle_events end\n");
}

int main(int argc, const char** argv)
{
	int fd, wd, poll_num;

	if(argc <2)
		printf("Usage: %s <path/file>\n", argv[0]),exit(-1);
	
	printf("notify [%s]\n",argv[1]);
	if( (fd = inotify_init()) == -1)
		perror("inotify_init"), exit(-1);

	if( (wd = inotify_add_watch(fd, argv[1], IN_MODIFY)) == -1)
		perror("inotify_add_watch"),exit(-1);

	struct pollfd fds;
	fds.fd = fd;
	fds.events = POLLIN;
	
	printf("Listening for events.\n");
	while(1)
	{
		printf("poll fd[%d][%08x], events[%d][%08x]\n", fds.fd, fds.fd, fds.events, fds.events);
		poll_num = poll(&fds, 1, -1);
		if(poll_num == -1)
		{
			if(errno == EINTR)
				continue;
			perror("poll"), exit(-1);
		}
		printf("poll_num [%d]\n", poll_num);
		if(poll_num >0)
		{
			printf("events [%d]\n", fds.revents);
			if(fds.revents & POLLIN)
			{
				handle_events(fd, wd, argc, argv);	
			}
		}
	}

	printf("Listening for events stopped. \n");

	close(fd);
	exit(0);
}
