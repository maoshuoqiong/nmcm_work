LOCAL_PATH := $(call my-dir)

include ${CLEAR_VARS}

LOCAL_MODULE := copyfile

LOCAL_LDLIBS += -llog

LOCAL_SRC_FILES := copyfile.c

include ${BUILD_EXECUTABLE}

