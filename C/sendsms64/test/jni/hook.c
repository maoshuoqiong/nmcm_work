#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/user.h>
#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <android/log.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <elf.h>

#define LOGE(fmt,args...) \
	__android_log_print( ANDROID_LOG_ERROR, "HOOKTEST", fmt, ##args)


#define CPSR_T_MASK  ( 1u << 5 )

static void print_register(struct user_pt_regs * regs)
{
	if(regs)
	{
		LOGE("+++++++++++ Register +++++++++++++");
		for(int i=0;i<31;i++)
			LOGE("[-] Register R[%d] = [%llx]",i,regs->regs[i]);
		
		LOGE("[-] Register sp= [0x%llx]", regs->sp);
		LOGE("[-] Register pc= [0x%llx]", regs->pc);
		LOGE("[-] Register pstate= [0x%llx]", regs->pstate);
		LOGE("++++++++++++++ End +++++++++++++++");
	}
}

void * get_module_base(pid_t pid, const char* module_name)
{
	FILE* fp;
	long addr =0;
	char *pch;
	char filename[32];
	char line[1024];
	if(pid<0)
		snprintf(filename, sizeof(filename), "/proc/self/maps");
	else
		snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);

	fp = fopen(filename, "r");
	if(fp != NULL)
	{
		while(fgets(line, sizeof(line),fp))
		{
			if(strstr(line, module_name))
			{
				pch = strtok(line, "-");
				addr = strtoull(pch, NULL, 16 );
				if(addr == 0x8000)
					addr = 0;
				break;
			}
		}
		fclose(fp);
	}
	
	return (void*)addr;	
}

void * get_remote_addr(pid_t target_pid, const char* module_name, void * local_addrs)
{
	void* local_handle, *remote_handle;
	local_handle = get_module_base(-1, module_name);
	remote_handle = get_module_base(target_pid, module_name);
	LOGE("[+] get_remote_addr: local[%p], remote[%p], local_func[%p]",local_handle, remote_handle, local_addrs);

	void *ret_addr = remote_handle + (local_addrs-local_handle);
	return ret_addr;
}

int ptrace_writedata(pid_t pid, uint8_t *dest, uint8_t* data, size_t size)
{
	long i, j, remain;
	uint8_t* laddr;
	const size_t bytes_width = sizeof(long);
	
	union u{
		long val;
		char chars[bytes_width];
	} d;	

	j = size/bytes_width;
	remain = size % bytes_width;

	laddr = data;
	
	for(i=0; i<j; i++)
	{
		memcpy(d.chars, laddr, bytes_width);
		ptrace(PTRACE_POKETEXT, pid, dest, d.val);
		
		dest += bytes_width;
		laddr+= bytes_width;
	}

	if(remain >0)
	{
		d.val = ptrace(PTRACE_PEEKTEXT, pid, dest, 0);
		for(i=0;i<remain; i++)
			d.chars[i] = *laddr++;

		ptrace(PTRACE_POKETEXT, pid, dest, d.val);
	}
	return 0;
}

int ptrace_continue(pid_t pid)
{
	if(ptrace(PTRACE_CONT, pid, NULL, 0) <0)
	{
		perror("ptrace_cont");
		return -1;
	}
	return 0;
}

int ptrace_getregs(pid_t pid, struct user_pt_regs *regs)
{
	/* long regset = NT_PRSTATUS; */
	struct iovec ioVec;
	ioVec.iov_base = regs;
	ioVec.iov_len = sizeof(*regs);

	/* i386 or arm
	* ptrace(PTRACE_GETREGS, pid NULL, regs)
	*/

	if(ptrace(PTRACE_GETREGSET, pid, (void*)NT_PRSTATUS, &ioVec)<0)
	{
		perror("ptrace getregset error");
		printf("io %p, %lu", ioVec.iov_base, ioVec.iov_len);
		return -1;
	}

	return 0;
}

int ptrace_setregs(pid_t pid, struct user_pt_regs *regs)
{
	int regset = NT_PRSTATUS;
	struct iovec ioVec;
	ioVec.iov_base = regs;
	ioVec.iov_len = sizeof(*regs);
	
	/* i386 or arm
		ptrace(PTRACE_SETREGS, pid, NULL, regs);
	*/
	if(ptrace(PTRACE_SETREGSET, pid, (void*)NT_PRSTATUS, &ioVec)<0)
	{
		perror("ptrace SETREGSET error:");
		return -1;
	}

	return 0;
}

int ptrace_call(pid_t pid, void* addr, long *params, int num_params, struct user_pt_regs* regs)
{
	int i;
#if defined(__arm__)
	int num_param_registers = 4;
#elif defined(__aarch64__)
	int num_param_registers = 8;
#endif

	for(i = 0; i< num_params && i< num_param_registers; i++)
		regs->regs[i] = params[i];

	if(i<num_params)
	{
		regs->sp -= (num_params - i) * sizeof(long);
		ptrace_writedata(pid, (uint8_t*)regs->sp, (uint8_t*)&params[i], (num_params - i) * sizeof(long)); 
	}
	
	regs->pc = (uint64_t)addr;
	if(regs->pc & 1)
	{
		/* thumb */
		regs->pc &= (~1u);
		regs->pstate |= CPSR_T_MASK;
	}
	else
	{
		/* arm */
		regs->pstate &= ~CPSR_T_MASK;
	}

	regs->regs[30] = 0;
	
	if(ptrace_setregs(pid, regs) == -1 )
	{
		LOGE("[+] set regs Error");
		return -1;
	}

	if( ptrace_continue(pid) == -1)
	{
		LOGE("[+] Error");
		return -1;
	}

	/* print_register(regs); */

	int stat = -1;
	pid_t wait_pid = waitpid(pid, &stat, WUNTRACED);
	if(wait_pid == -1)
	{
		int err = errno;	
		LOGE("[+] waitpid error[%d], %s",err, strerror(err));
		if(err == EACCES)
			return 0;
	}
	LOGE("[+] pstarce_call stat:[%d]:[%x], return pid:[%d]",stat,stat,wait_pid); 

	while(stat != 0xb7f)
	{
		if(ptrace_continue(pid) == -1)
		{
			LOGE("Error stat");
			return -1;
		}
		waitpid(pid, &stat, WUNTRACED);
		LOGE("[+] pstarce_call stat:[%d]",stat); 
	}

	return 0;
}

uint64_t ptrace_retval(struct user_pt_regs *regs)
{
	/* return regs->eax; */
	return regs->regs[0];
}

uint64_t ptrace_ip(struct user_pt_regs *regs)
{
	/* return regs->eip; */
	return regs->pc;
}

int ptrace_call_wrapper(pid_t target_pid, const char* func_name, void *func_addr, long *parameters, int param_num, struct user_pt_regs * regs)
{
	LOGE("[+] Calling %s in target process.", func_name);
	if(ptrace_call(target_pid, func_addr, parameters, param_num, regs) == -1)
		return -1;

	if(ptrace_getregs(target_pid, regs) == -1)
		return -1;
	LOGE("[+] Target process returned from %s, return value = %lx, pc=%lx",func_name, ptrace_retval(regs), ptrace_ip(regs));
	return 0;
}

int main(int argc, const char *argv[])
{
	uid_t uid=0,euid=0;
	
	LOGE("uid[%d], euid[%d]",getuid(), geteuid());

	if(argc != 2)
		printf("usage: %s <pid>\n",argv[0]),exit(-1);

	pid_t trace_pid = atoi(argv[1]);

	if(ptrace(PTRACE_ATTACH, trace_pid, NULL, NULL)==-1)
		perror("PTRACE_ATTACH error"),exit(-1);

	if(waitpid(trace_pid, NULL, WUNTRACED)!=trace_pid)
		perror("waitpid error");
	LOGE("attach success");

	struct user_pt_regs regs,regs_bak;

	if(ptrace_getregs(trace_pid, &regs) == -1)
		goto exit;

	memcpy(&regs_bak, &regs, sizeof(regs));
	/* print_register(&regs_bak); */

	void *p_addr = get_remote_addr(trace_pid, "/system/lib64/libc.so", &mmap);
	LOGE("[+] Remote mmap address: %p", p_addr);

	long parameters[10];
	parameters[0] = 0; 
	parameters[1] = 0x4000;
	parameters[2] = PROT_READ | PROT_WRITE | PROT_EXEC;
	parameters[3] = MAP_ANONYMOUS | MAP_PRIVATE;
	parameters[4] = 0;
	parameters[5] = 0;

	void * map_addr = NULL;	

	if(ptrace_call_wrapper( trace_pid, "mmap", p_addr, parameters, 6, &regs) == -1)
	{
		LOGE("[+] call remote mmap error");
		goto exit2;
	}

	map_addr = (void*)ptrace_retval(&regs);
	LOGE("mmaped address is [%p]",map_addr);

	printf("Enter any key to detach\n");
	getchar();

exit2:
	ptrace_setregs(trace_pid, &regs_bak);

exit:
	
	if(ptrace(PTRACE_DETACH, trace_pid, NULL, NULL)==-1)
		perror("PTRACE_DETACH error"),exit(-1);
	LOGE("detach success");
		
	return 0;
}

