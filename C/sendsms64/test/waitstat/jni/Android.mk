LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := waitstat
LOCAL_SRC_FILES := waitstat.c

include $(BUILD_EXECUTABLE)

