LOCAL_PATH := $(call my-dir)  

include $(CLEAR_VARS)

LOCAL_MODULE := sqlitetest

LOCAL_LDLIBS += -llog -ldl

LOCAL_SRC_FILES := sqlitetest.c

#LOCAL_CFLAGS += -g
#LOCAL_CPPFLAGS += -g

# no effective
#LOCAL_STRIP_MODULE += false 

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := sym_sqlite
LOCAL_LDLIBS := -llog -ldl
LOCAL_SRC_FILES := sym_sqlite.c

LOCAL_CFLAGS += -fPIC

include $(BUILD_SHARED_LIBRARY)

