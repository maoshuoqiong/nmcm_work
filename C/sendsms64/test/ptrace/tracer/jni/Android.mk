LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := tracer

LOCAL_SRC_FILES := tracer.c main.c

LOCAL_LDLIBS := -llog

include $(BUILD_EXECUTABLE)

