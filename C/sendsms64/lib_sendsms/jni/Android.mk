LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)

LOCAL_MODULE := sm

#LOCAL_MODULE_FILENAME := libsm
  
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog -ldl

MY_CPP_LIST := $(wildcard $(LOCAL_PATH)/work.c)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/sendmessage.c)

LOCAL_SRC_FILES := $(MY_CPP_LIST:$(LOCAL_PATH)/%=%)

LOCAL_CFLAGS += -DZNIU_BUDEG=1
LOCAL_CFLAGS += -DHEIGHT_VERSION=1
LOCAL_CFLAGS += -fvisibility=hidden

include $(BUILD_SHARED_LIBRARY)
