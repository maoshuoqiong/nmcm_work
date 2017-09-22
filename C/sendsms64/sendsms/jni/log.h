#ifndef __LOG_H__
#define __LOG_H__

#include <android/log.h>

#if ZNIU_BUDEG
    #define  LOGI(fmt, args...) \
    __android_log_print( ANDROID_LOG_INFO, "HOOK", fmt, ##args)

    #define  LOGD(fmt, args...) \
    __android_log_print( ANDROID_LOG_DEBUG, "HOOK", fmt, ##args)

    #define  LOGE(fmt, args...) \
    __android_log_print( ANDROID_LOG_ERROR, "HOOK", fmt, ##args)
#else
    #define LOGI(F,...)
    #define LOGD(F,...)
    #define LOGE(F,...)
#endif

#endif //__LOG_H__
