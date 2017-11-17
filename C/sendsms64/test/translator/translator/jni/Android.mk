LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := translator
LOCAL_SRC_FILES := translator.c
LOCAL_LDLIBS := -llog

include $(BUILD_EXECUTABLE)

