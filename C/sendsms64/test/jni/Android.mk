LOCAL_PATH := $(call my-dir)  

include $(CLEAR_VARS)

LOCAL_MODULE := inotify

#LOCAL_LDLIBS += -llog -ldl

LOCAL_SRC_FILES := inotify.c

#LOCAL_CFLAGS += -g
#LOCAL_CPPFLAGS += -g

# no effective
#LOCAL_STRIP_MODULE += false 

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)

