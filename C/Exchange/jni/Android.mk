LOCAL_PATH := $(call my-dir)  

include $(CLEAR_VARS)

LOCAL_MODULE := mbstowcs

LOCAL_LDLIBS += -llog 

#LOCAL_SRC_FILES := mbstowcs.c
LOCAL_SRC_FILES := ori.c


LOCAL_CFLAGS += -g
LOCAL_CPPFLAGS += -g

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)

