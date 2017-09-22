LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)

LOCAL_MODULE :=sendsms

LOCAL_MODULE_FILENAME :=sendsms
  
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog   

MY_CPP_LIST := $(wildcard $(LOCAL_PATH)/main.c)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/hook.c)



LOCAL_SRC_FILES := $(MY_CPP_LIST:$(LOCAL_PATH)/%=%)

LOCAL_CFLAGS += -DZNIU_BUDEG=1

LOCAL_CFLAGS += -fvisibility=hidden

LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE

 
#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)
