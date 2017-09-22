LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)
LOCAL_MODULE    := dalvik
#LOCAL_SRC_FILES := ../../libdalvik/obj/local/arm64-v8a/libdalvikhook.a
LOCAL_SRC_FILES := ../../libdalvik/obj/local/armeabi-v7a/libdalvikhook.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := sm

#LOCAL_MODULE_FILENAME := libsm
  
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog

MY_CPP_LIST := $(wildcard $(LOCAL_PATH)/work.c)

LOCAL_SRC_FILES := $(MY_CPP_LIST:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../libdalvik/jni \


LOCAL_STATIC_LIBRARIES := dalvik

LOCAL_CFLAGS += -DZNIU_BUDEG=1
LOCAL_CFLAGS += -DHEIGHT_VERSION=1
LOCAL_CFLAGS += -fvisibility=hidden

include $(BUILD_SHARED_LIBRARY)
