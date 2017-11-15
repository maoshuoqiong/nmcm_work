LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := hook

LOCAL_LDLIBS += -llog -ldl

LOCAL_SRC_FILES := hook.c

include $(BUILD_SHARED_LIBRARY)

