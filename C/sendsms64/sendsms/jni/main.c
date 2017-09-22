
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
#include "hook.h"
#include "log.h"

int main(int arcv,char * argv[]){
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



