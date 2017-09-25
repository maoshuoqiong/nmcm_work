LOCAL_PATH := $(call my-dir)  

include $(CLEAR_VARS)

LOCAL_MODULE := work

LOCAL_LDLIBS += -llog -ldl

LOCAL_SRC_FILES := work.c

LOCAL_CFLAGS += -g
LOCAL_CPPFLAGS += -g

# no effective
#LOCAL_STRIP_MODULE += false 

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := hook

LOCAL_LDLIBS += -llog -ldl

LOCAL_SRC_FILES := hook.c

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)
