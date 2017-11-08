LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libdalvikhook

#LOCAL_SHARED_LIBRARIES := -L$(SYSROOT)/usr/lib -L./libs -lart -ldl  -llog     #ldvm  lart
#LOCAL_LDLIBS    := -L$(SYSROOT)/usr/lib -L./libs -lart -ldl  -llog     #ldvm  lart
LOCAL_SHARED_LIBRARIES := -ldl  -llog     #ldvm  lart

LOCAL_SRC_FILES := dexstuff.c dalvik_hook.c

LOCAL_CFLAGS += -DZNIU_BUDEG=1
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
	LOCAL_CFLAGS += -DART
endif

#LOCAL_CFLAGS += -fvisibility=hidden
 
include $(BUILD_STATIC_LIBRARY)
