#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <dlfcn.h>

extern int hook_entry(char* a);
typedef int hook_entry_method(char*);

typedef int P(const char* , ...);

void * getModuleAddr(const char* module_name)
{
	FILE *fp;
    long addr = 0;
    char *pch;
    char filename[32];
    char line[1024];

    snprintf(filename, sizeof(filename), "/proc/self/maps");
    fp = fopen(filename, "r");

    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name)) {
                pch = strtok( line, "-" );
                addr = strtoull( pch, NULL, 16 );

                if (addr == 0x8000)
                    addr = 0;

                break;
            }
        }

        fclose(fp) ;
    }

    return (void *)addr;
}

int main(void)
{
	printf("mmap:[%p]\n",&mmap);
	printf("sleep:[%p]\n",&sleep);
	printf("printf:[%p]\n",&printf);
	printf("hook_entry:[%p]\n",*hook_entry);

	hook_entry("Haha");
/*
	P* p = (P*)0x400490;
	p("Hello world\n");	
*/
	
	void* p = (hook_entry_method*)getModuleAddr("libhello.so");
	if(p==NULL)
	{
		printf("get libhello.so address failed\n");
		return -1;
	}
	printf("library address is [%p]\n",p);
	hook_entry_method *p_m = p+0x6f0;
	printf("hook_entry p:[%p]\n",p_m);

	p_m("method");

	void *p1 = dlopen("./libhello.so",RTLD_LAZY);
	if(p1 == NULL)
	{
		printf("dlopen error: %s\n",dlerror());
		return -1;
	}

	printf("dlopen library address is [%p]\n",p1);
	void* p2 = dlsym(p1,"hook_entry");
	printf("p2 address is [%p]\n",p2);

	dlclose(p1);
	
	int i=0;
	while(i<10000)
	{
		printf("%d\n",i++);
		sleep(1);
	}


	return 0;
}

