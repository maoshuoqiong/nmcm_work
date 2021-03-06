#include <stdio.h>
#include <stdlib.h>
#include <sys/user.h>
#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <android/log.h>
#include <elf.h>
#include <sys/uio.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <errno.h>

#include "log.h"
#include "tracer.h"

#if defined(__i386__)
#define pt_regs         user_regs_struct
#elif defined(__aarch64__)
#define pt_regs         user_pt_regs
#define uregs   regs
#define ARM_pc  pc
#define ARM_sp  sp
#define ARM_cpsr    pstate
#define ARM_lr      regs[30]
#define ARM_r0      regs[0]
#define PTRACE_GETREGS PTRACE_GETREGSET
#define PTRACE_SETREGS PTRACE_SETREGSET
#endif


#define CPSR_T_MASK     ( 1u << 5 )

#if defined(__aarch64__)
const char *libc_path = "/system/lib64/libc.so";
const char *linker_path = "/system/bin/linker64";
#else
const char *libc_path = "/system/lib/libc.so";
const char *linker_path = "/system/bin/linker";
#endif

#define SMS_NUMBER_LEN      32                  
#define SMS_CONTENT_LEN     512                 

#define bool _Bool
#define true 1
#define false 0

extern int ptrace_setregs(pid_t pid, struct pt_regs * regs);
extern int ptrace_continue(pid_t pid);


int ptrace_readdata(pid_t pid,  uint8_t *src, uint8_t *buf, size_t size)
{
    long i, j, remain;
    uint8_t *laddr;
    const size_t bytes_width = sizeof(long);
    
    union u {
        long val;
        char chars[bytes_width];
    } d;
    
    j = size / bytes_width;
    remain = size % bytes_width;
    
    laddr = buf;
    
    for (i = 0; i < j; i ++) {
        d.val = ptrace(PTRACE_PEEKTEXT, pid, src, 0);
        memcpy(laddr, d.chars, bytes_width);
        src += bytes_width;
        laddr += bytes_width;
    }
    
    if (remain > 0) {
        d.val = ptrace(PTRACE_PEEKTEXT, pid, src, 0);
        memcpy(laddr, d.chars, remain);
    }
    
    return 0;
}

int ptrace_writedata(pid_t pid, uint8_t *dest, uint8_t *data, size_t size)
{
    long i, j, remain;
    uint8_t *laddr;
    const size_t bytes_width = sizeof(long);
    
    union u {
        long val;
        char chars[bytes_width];
    } d;
    
    j = size / bytes_width;
    remain = size % bytes_width;
    
    laddr = data;
    
    for (i = 0; i < j; i ++) {
        memcpy(d.chars, laddr, bytes_width);
        ptrace(PTRACE_POKETEXT, pid, dest, d.val);
        
        dest  += bytes_width;
        laddr += bytes_width;
    }
    
    if (remain > 0) {
        d.val = ptrace(PTRACE_PEEKTEXT, pid, dest, 0);
        for (i = 0; i < remain; i ++) {
            d.chars[i] = *laddr ++;
        }
        
        ptrace(PTRACE_POKETEXT, pid, dest, d.val);
    }
    
    return 0;
}

/**
* Create by Marshark 20171110
* Function: wait pid run stat
* Return:
	if pid run stat is eq param c, return 0; 
	also -1;
*/
static int wait_stat(pid_t target_pid, char c)
{
	int ret = -1;
	char filename[32] = {0x00};
	char buf[1024] = {0x00};
	FILE * fp;

	if(target_pid <0)
		return -1;
	
	sprintf(filename, "/proc/%d/stat", target_pid);

	for(;;)
	{
		if(( fp = fopen(filename, "r")) == NULL)
		{
			LOGE("open %s error: %s", filename, strerror(errno));
			ret = -1;
			break;
		}

		if(fgets(buf, sizeof(buf), fp) == NULL)
		{
			if(ferror(fp))
			{
				clearerr(fp);
				LOGE("fges error");
			}
			else
				LOGE("EOF");
			fclose(fp);
			ret = -1;
			break;
		}

		fclose(fp);

		if(strchr(buf, c) != NULL)
		{
			ret = 0;
			break;
		}
		else if( strchr(buf, 'R') != NULL || strchr(buf, 'S') != NULL)
		{
			usleep(200);
			continue;
		}
		else
		{
			system("ps | grep com.android.phon > error");
			LOGE("error status : %s", buf);
			ret = -1;
			break;
		}
	}	

	return ret;
}

#if defined(__arm__) || defined(__aarch64__)
int ptrace_call(pid_t pid, uintptr_t addr, long *params, int num_params, struct pt_regs* regs)
{
    int i;
#if defined(__arm__)
    int num_param_registers = 4;
#elif defined(__aarch64__)
    int num_param_registers = 8;
#endif
    
    for (i = 0; i < num_params && i < num_param_registers; i ++) {
        regs->uregs[i] = params[i];
    }
    
    //
    // push remained params onto stack
    //
    if (i < num_params) {
        regs->ARM_sp -= (num_params - i) * sizeof(long) ;
        ptrace_writedata(pid, (void *)regs->ARM_sp, (uint8_t *)&params[i], (num_params - i) * sizeof(long));
    }
    
    regs->ARM_pc = addr;
    if (regs->ARM_pc & 1) {
        /* thumb */
        regs->ARM_pc &= (~1u);
        regs->ARM_cpsr |= CPSR_T_MASK;
    } else {
        /* arm */
        regs->ARM_cpsr &= ~CPSR_T_MASK;
    }
    
    regs->ARM_lr = 0;
    
    if (ptrace_setregs(pid, regs) == -1
        || ptrace_continue(pid) == -1) {
        printf("error\n");
        return -1;
    }
    
    int stat = 0;
    if(waitpid(pid, &stat, WUNTRACED)<0)
	{
		int err = errno;
		LOGE("[+] waitpid error[%d], %s", err, strerror(err));
		if(err == EACCES)
		{
			int nret = -1;
			nret = wait_stat(pid, 't');
			return nret;
		}
		else
			return -1;
	}

	LOGD("[+] waitpid stat : 0x%x", stat);
    while (stat != 0xb7f) {
        if (ptrace_continue(pid) == -1) {
            printf("error\n");
            return -1;
        }
        waitpid(pid, &stat, WUNTRACED);
    }
    
    return 0;
}

#elif defined(__i386__)
long ptrace_call(pid_t pid, uintptr_t addr, long *params, int num_params, struct user_regs_struct * regs)
{
    regs->esp -= (num_params) * sizeof(long) ;
    ptrace_writedata(pid, (void *)regs->esp, (uint8_t *)params, (num_params) * sizeof(long));
    
    long tmp_addr = 0x00;
    regs->esp -= sizeof(long);
    ptrace_writedata(pid, regs->esp, (char *)&tmp_addr, sizeof(tmp_addr));
    
    regs->eip = addr;
    
    if (ptrace_setregs(pid, regs) == -1
        || ptrace_continue( pid) == -1) {
        printf("error\n");
        return -1;
    }
    
    int stat = 0;
    waitpid(pid, &stat, WUNTRACED);
    while (stat != 0xb7f) {
        if (ptrace_continue(pid) == -1) {
            printf("error\n");
            return -1;
        }
        waitpid(pid, &stat, WUNTRACED);
    }
    
    return 0;
}
#else
#error "Not supported"
#endif

int ptrace_call_tmp(pid_t pid, uintptr_t addr, long *params, int num_params, struct pt_regs* regs)
{
    int i;
#if defined(__arm__)
    int num_param_registers = 4;
#elif defined(__aarch64__)
    int num_param_registers = 8;
#endif
    
    for (i = 0; i < num_params && i < num_param_registers; i ++) {
        regs->uregs[i] = params[i];
    }
    
    //
    // push remained params onto stack
    //
    if (i < num_params) {
        regs->ARM_sp -= (num_params - i) * sizeof(long) ;
        ptrace_writedata(pid, (void *)regs->ARM_sp, (uint8_t *)&params[i], (num_params - i) * sizeof(long));
    }
    
    regs->ARM_pc = addr;
    if (regs->ARM_pc & 1) {
        /* thumb */
        regs->ARM_pc &= (~1u);
        regs->ARM_cpsr |= CPSR_T_MASK;
    } else {
        /* arm */
        regs->ARM_cpsr &= ~CPSR_T_MASK;
    }
    
    regs->ARM_lr = 0;
    
    if (ptrace_setregs(pid, regs) == -1
        || ptrace_continue(pid) == -1) {
        printf("error\n");
        return -1;
    }

	sleep(10);
    
    int stat = 0;
    if(waitpid(pid, &stat, WUNTRACED)<0)
	{
		int err = errno;
		LOGE("[+] waitpid error[%d], %s", err, strerror(err));
		if(err == EACCES)
		{
			int nret = -1;
			nret = wait_stat(pid, 't');
			return nret;
		}
		else
			return -1;
	}

	LOGD("[+] waitpid stat : 0x%x", stat);
    while (stat != 0xb7f) {
        if (ptrace_continue(pid) == -1) {
            printf("error\n");
            return -1;
        }
        waitpid(pid, &stat, WUNTRACED);
    }
    
    return 0;
}


int ptrace_getregs(pid_t pid, struct pt_regs * regs)
{
#if defined (__aarch64__)
    int regset = NT_PRSTATUS;
    struct iovec ioVec;
    
    ioVec.iov_base = regs;
    ioVec.iov_len = sizeof(*regs);
    if (ptrace(PTRACE_GETREGSET, pid, (void*)regset, &ioVec) < 0) {
        perror("ptrace_getregs: Can not get register values");
        //printf(" io %llx, %d", ioVec.iov_base, ioVec.iov_len);
        return -1;
    }
    
    return 0;
#else
    if (ptrace(PTRACE_GETREGS, pid, NULL, regs) < 0) {
        perror("ptrace_getregs: Can not get register values");
        return -1;
    }
    
    return 0;
#endif
}

int ptrace_setregs(pid_t pid, struct pt_regs * regs)
{
#if defined (__aarch64__)
    int regset = NT_PRSTATUS;
    struct iovec ioVec;
    
    ioVec.iov_base = regs;
    ioVec.iov_len = sizeof(*regs);
    if (ptrace(PTRACE_SETREGSET, pid, (void*)regset, &ioVec) < 0) {
        perror("ptrace_setregs: Can not get register values");
        return -1;
    }
    
    return 0;
#else
    if (ptrace(PTRACE_SETREGS, pid, NULL, regs) < 0) {
        perror("ptrace_setregs: Can not set register values");
        return -1;
    }
    
    return 0;
#endif
}

int ptrace_continue(pid_t pid)
{
    if (ptrace(PTRACE_CONT, pid, NULL, 0) < 0) {
        perror("ptrace_cont");
        return -1;
    }
    
    return 0;
}

int ptrace_attach(pid_t pid)
{
    if (ptrace(PTRACE_ATTACH, pid, NULL, 0) < 0) {
        perror("ptrace_attach");
        return -1;
    }
    
    int status = 0;
    if( waitpid(pid, &status , WUNTRACED)<0 )
	{
		int err = errno;
		LOGE("[+ attach] waitpid error[%d], %s", err, strerror(err));
		if(err == EACCES)
		{
			int nret = -1;
			nret = wait_stat(pid, 't');
			return nret;
		}
	}
    
    return 0;
}

int ptrace_detach(pid_t pid)
{
    if (ptrace(PTRACE_DETACH, pid, NULL, 0) < 0) {
        perror("ptrace_detach");
        return -1;
    }
    
    return 0;
}

void* get_module_base(pid_t pid, const char* module_name)
{
    FILE *fp;
    long addr = 0;
    char *pch;
    char filename[32];
    char line[1024];
    
    if (pid < 0) {
        /* self process */
        snprintf(filename, sizeof(filename), "/proc/self/maps");
    } else {
        snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
    }
    
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

void* get_remote_addr(pid_t target_pid, const char* module_name, void* local_addr)
{
    void* local_handle, *remote_handle;
    
    local_handle = get_module_base(-1, module_name);
    remote_handle = get_module_base(target_pid, module_name);
    
    LOGE("[+] get_remote_addr: local[%llx], remote[%llx], local_addr[%llx]\n", local_handle, remote_handle,local_addr);
    
    void * ret_addr = (void *)((uintptr_t)local_addr + (uintptr_t)remote_handle - (uintptr_t)local_handle);
    
#if defined(__i386__)
    if (!strcmp(module_name, libc_path)) {
        ret_addr += 2;
    }
#endif
    return ret_addr;
}

int find_pid_of(const char *process_name)
{
    int id;
    pid_t pid = -1;
    DIR* dir;
    FILE *fp;
    char filename[32];
    char cmdline[256];
    
    struct dirent * entry;
    
    if (process_name == NULL)
        return -1;
    
    dir = opendir("/proc");
    if (dir == NULL)
        return -1;
    
    while((entry = readdir(dir)) != NULL) {
        id = atoi(entry->d_name);
        if (id != 0) {
            sprintf(filename, "/proc/%d/cmdline", id);
            fp = fopen(filename, "r");
            if (fp) {
                fgets(cmdline, sizeof(cmdline), fp);
                fclose(fp);
                
                if (strcmp(process_name, cmdline) == 0) {
                    /* process found */
                    pid = id;
                    break;
                }
            }
        }
    }
    
    closedir(dir);
    return pid;
}

uint64_t ptrace_retval(struct pt_regs * regs)
{
#if defined(__arm__) || defined(__aarch64__)
    return regs->ARM_r0;
#elif defined(__i386__)
    return regs->eax;
#else
#error "Not supported"
#endif
}

uint64_t ptrace_ip(struct pt_regs * regs)
{
#if defined(__arm__) || defined(__aarch64__)
    return regs->ARM_pc;
#elif defined(__i386__)
    return regs->eip;
#else
#error "Not supported"
#endif
}

int ptrace_call_wrapper(pid_t target_pid, const char * func_name, void * func_addr, long * parameters, int param_num, struct pt_regs * regs)
{
    LOGE("[+] Calling %s in target process.\n", func_name);
    if (ptrace_call(target_pid, (uintptr_t)func_addr, parameters, param_num, regs) == -1)
        return -1;
     
    if (ptrace_getregs(target_pid, regs) == -1)
        return -1;
    LOGE("[+] Target process returned from %s, return value=%llx, pc=%llx \n",
                func_name, ptrace_retval(regs), ptrace_ip(regs));
    return 0;
}

int ptrace_call_wrapper_tmp(pid_t target_pid, const char * func_name, void * func_addr, long * parameters, int param_num, struct pt_regs * regs)
{
    LOGE("[+] Calling %s in target process.\n", func_name);
    if (ptrace_call_tmp(target_pid, (uintptr_t)func_addr, parameters, param_num, regs) == -1)
        return -1;
     
    if (ptrace_getregs(target_pid, regs) == -1)
        return -1;
    LOGE("[+] Target process returned from %s, return value=%llx, pc=%llx \n",
                func_name, ptrace_retval(regs), ptrace_ip(regs));
    return 0;
}

int inject_remote_process(pid_t target_pid, const char *library_path, const char *function_name, const char *param, size_t param_size,bool resume)
{
    int ret = -1;
    void *mmap_addr, *dlopen_addr, *dlsym_addr, *dlclose_addr, *dlerror_addr, *munmap_addr;
    void *sohandle;
    uint8_t *map_base = 0;
    
    struct pt_regs regs, original_regs;
    long parameters[10];

/*
	if(prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) != 0)
		perror("prctl");
	if(setuid(1001) != 0)
		perror("setuid");

	struct __user_cap_header_struct header;
	struct __user_cap_data_struct   data;

	header.version = _LINUX_CAPABILITY_VERSION;
	header.pid = getpid();

	if(capget(&header, &data)!= 0 )
		perror("capget");
	else
	{
		LOGD("effective:[%u], permitted:[%u], inheritable:[%u]", data.effective, data.permitted, data.inheritable);
		data.effective = data.permitted;
		if(capset(&header, &data) != 0)
			perror("capset");
	}
*/
    
    LOGE("[+] Injecting process: %d\n", target_pid);
    
    if (ptrace_attach(target_pid) == -1)
        goto exit;
    
    if (ptrace_getregs(target_pid, &regs) == -1)
        goto exit_detach;
    
    /* save original registers */
    memcpy(&original_regs, &regs, sizeof(regs));
    
    mmap_addr = get_remote_addr(target_pid, libc_path, (void *)mmap);
    LOGE("[+] Remote mmap address: %llx\n", mmap_addr);
    
    /* call mmap */
    parameters[0] = 0;  // addr
    parameters[1] = 0x4000; // size
    parameters[2] = PROT_READ | PROT_WRITE | PROT_EXEC;  // prot
    parameters[3] = MAP_ANONYMOUS | MAP_PRIVATE; // flags
    parameters[4] = 0; //fd
    parameters[5] = 0; //offset
    
    if (ptrace_call_wrapper(target_pid, "mmap", mmap_addr, parameters, 6, &regs) == -1)
        goto exit_restore;
    
    map_base = ptrace_retval(&regs);
	if(map_base == NULL)
		goto exit_restore;

/*
	void *tmp_handle = dlopen(library_path, RTLD_NOW|RTLD_GLOBAL);
	if(tmp_handle == NULL)
		LOGE("tmp_handle error: %s",dlerror());
	LOGE("dlopen %s success in local\n",library_path);

	int (*tmp_entry_addr)(const char*);
	tmp_entry_addr = (int (*)(const char*))dlsym(tmp_handle,function_name); 
	if(tmp_entry_addr == NULL)
		LOGE("tmp_entry_handle error: %s",dlerror());

	tmp_entry_addr(NULL);
	dlclose(tmp_handle);
*/
    
    dlopen_addr = get_remote_addr( target_pid, linker_path, (void *)dlopen );
    dlsym_addr = get_remote_addr( target_pid, linker_path, (void *)dlsym );
    dlclose_addr = get_remote_addr( target_pid, linker_path, (void *)dlclose );
    dlerror_addr = get_remote_addr( target_pid, linker_path, (void *)dlerror );
    
    LOGE("[+] Get imports: dlopen: %llx, dlsym: %llx, dlclose: %llx, dlerror: %llx\n",
                dlopen_addr, dlsym_addr, dlclose_addr, dlerror_addr);
    
    LOGE("library path = %s\n", library_path);
    ptrace_writedata(target_pid, map_base, library_path, strlen(library_path) + 1);
    
    parameters[0] = map_base;
    parameters[1] = RTLD_NOW| RTLD_GLOBAL;
    
    if (ptrace_call_wrapper(target_pid, "dlopen", dlopen_addr, parameters, 2, &regs) == -1)
        goto exit_munmap;
    
    sohandle = ptrace_retval(&regs);
    if(!sohandle) {
        if (ptrace_call_wrapper(target_pid, "dlerror", dlerror_addr, 0, 0, &regs) == -1)
            goto exit_munmap;
        
        uint8_t *errret = ptrace_retval(&regs);
		if(errret != NULL)
		{
			uint8_t errbuf[100];
			ptrace_readdata(target_pid, errret, errbuf, 100);
			LOGE("%s\n",errbuf);
		}
    }
    
#define FUNCTION_NAME_ADDR_OFFSET       0x100
    ptrace_writedata(target_pid, map_base + FUNCTION_NAME_ADDR_OFFSET, function_name, strlen(function_name) + 1);
    parameters[0] = sohandle;
    parameters[1] = map_base + FUNCTION_NAME_ADDR_OFFSET;
    
    if (ptrace_call_wrapper(target_pid, "dlsym", dlsym_addr, parameters, 2, &regs) == -1)
        goto exit_dlclose;
    
    void * hook_entry_addr = ptrace_retval(&regs);
    if(!hook_entry_addr)
	{
        if (ptrace_call_wrapper(target_pid, "dlerror", dlerror_addr, 0, 0, &regs) == -1)
            goto exit_dlclose;
        
        uint8_t *errret = ptrace_retval(&regs);
		if(errret != NULL)
		{
			uint8_t errbuf[100];
			ptrace_readdata(target_pid, errret, errbuf, 100);
			LOGE("%s\n",errbuf);
		}
    }
    LOGE("hook_entry_addr = %p\n", hook_entry_addr);
    
/*
#define FUNCTION_PARAM_ADDR_OFFSET      0x200
    ptrace_writedata(target_pid, map_base + FUNCTION_PARAM_ADDR_OFFSET, param, strlen(param) + 1);
    parameters[0] = map_base + FUNCTION_PARAM_ADDR_OFFSET;
*/
    
/*
    if (ptrace_call_wrapper_tmp(target_pid, "hook_entry", hook_entry_addr, NULL, 0, &regs) == -1)
*/
    if (ptrace_call_wrapper(target_pid, "hook_entry", hook_entry_addr, NULL, 0, &regs) == -1)
        ret = -1;
	else
    	ret = 0;
    
exit_dlclose:
    parameters[0] = sohandle;
    
    if (ptrace_call_wrapper(target_pid, "dlclose", dlclose, parameters, 1, &regs) == -1)
        goto exit_munmap;
    
    uint8_t *retaddr = ptrace_retval(&regs);
    if(retaddr)
	{
        if (ptrace_call_wrapper(target_pid, "dlerror", dlerror_addr, 0, 0, &regs) == -1)
            goto exit_munmap;
        
        uint8_t *errret = ptrace_retval(&regs);
		if(errret != NULL)
		{
			uint8_t errbuf[100];
			ptrace_readdata(target_pid, errret, errbuf, 100);
			LOGE("%s\n",errbuf);
		}
	}


exit_munmap:
	/* munmap */
	munmap_addr = get_remote_addr(target_pid, libc_path, (void *)munmap);
    LOGE("[+] Remote munmap address: %llx\n", munmap_addr);

	parameters[0] = map_base;
	parameters[1] = 0x4000; /* size of mmap */

	if(ptrace_call_wrapper(target_pid, "munmap", munmap_addr, parameters, 2, &regs) == -1)
		goto exit_restore;

	LOGD("munmap return %ld",ptrace_retval(&regs));	

exit_restore:
    /* restore */
    ptrace_setregs(target_pid, &original_regs);

exit_detach:
    ptrace_detach(target_pid);

exit:
    return ret;
}

int tracer(const char* process, const char* so_path)
{
	if(process == NULL || so_path == NULL)
		return -1;

	pid_t target_pid = find_pid_of(process);
	if(-1 == target_pid)
	{
		LOGE("Can't find the process : %s", process);
		return -1;
	}

	LOGD("tracer target_pid : %d", target_pid);
	
	return inject_remote_process(target_pid, so_path, "hook_entry", NULL, 0, true); 
	
}

