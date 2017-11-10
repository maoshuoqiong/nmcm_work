#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

static const char* PROCESS_NAME = "com.android.phone";

static pid_t find_pid_of(const char* process_name)
{
	int id;
	pid_t pid = -1;
	DIR* dir;
	FILE *fp;
	char filename[32];
	char cmdline[256];
	
	struct dirent *entry;
	
	if(process_name == NULL)
		return -1;

	dir = opendir("/proc");
	if(dir == NULL)
		return -1;
	
	while( (entry = readdir(dir)) != NULL)
	{
		id = atoi(entry->d_name);
		if(id != 0)
		{
			sprintf(filename, "/proc/%d/cmdline", id);
			fp = fopen(filename, "r");
			if(fp)
			{
				fgets(cmdline, sizeof(cmdline), fp);
				fclose(fp);

				if(strcmp(process_name, cmdline) == 0)
				{
					pid = id;
					break;
				}
			}
		}	
	}

	closedir(dir);
	return pid;
}

static int wait_stat(pid_t target_pid, char c)
{
	int ret = -1;
	char filename[32] = {0x00};
	char buf[4096] = {0x00};
	FILE * fp;

	if(target_pid < 0)
		return -1;

	sprintf(filename, "/proc/%d/stat", target_pid);

	for(;;)
	{
	
		if(( fp = fopen(filename, "r") ) == NULL)
		{
			printf("open %s error: %s\n", filename, strerror(errno));
			ret = -1;
			break;
		}

		if(fgets(buf, sizeof(buf), fp) == NULL)
		{
			if(ferror(fp))
			{
				clearerr(fp);
				printf("fgets error\n");
			}
			else
				printf("EOF!\n");
			fclose(fp);
			ret = -1;
			break;
		}

		fclose(fp);

		if(strchr(buf, c)!= NULL)	
		{
			ret = 0;
			break;
		}

		usleep(200);

	}

	return ret;
}

int main(int argc, const char* argv[])
{

	pid_t target_pid = find_pid_of(PROCESS_NAME);
	if(-1 == target_pid)
	{
		printf("can't find the process\n");
		return -1;
	}

	printf("pid: %d\n", target_pid);

	if(wait_stat(target_pid, 'R') == 0)
		printf("stat is S\n");
	else
		printf("error\n");
	return 0;
}

