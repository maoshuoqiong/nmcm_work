#ifndef __LOG_H_
#define __LOG_H_

#include <jni.h>
#include <android/log.h>

#define TAG "HOOKTEST"

#define LOGD(fmt, args...) \
	__android_log_print( ANDROID_LOG_DEBUG, TAG, fmt, ##args)

#define LOGE(fmt, args...) \
	__android_log_print( ANDROID_LOG_ERROR, TAG, fmt, ##args)

#endif

