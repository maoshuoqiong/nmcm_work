LOCAL_PATH := $(call my-dir)  

include $(CLEAR_VARS)

LOCAL_MODULE := pthreadtest

#LOCAL_LDLIBS += -llog -ldl
LOCAL_LDLIBS := -pthread

LOCAL_SRC_FILES := pthreadtest.c

#LOCAL_CFLAGS += -g
#LOCAL_CPPFLAGS += -g

# no effective
#LOCAL_STRIP_MODULE += false 

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)

