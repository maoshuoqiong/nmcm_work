#ifndef __HOOK_H__
#define __HOOK_H__



#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
    

/**
 * hook send text sms
 *
 *
 */
int hook_send_text_sms(char * number,char * content);

/**
 * hook send text sms
 *
 *
 */
int hook_send_data_sms(char * number,char * content,uint32_t port);
    

    
    
#ifdef __cplusplus
}
#endif

#endif
