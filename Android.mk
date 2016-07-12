LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libmirror
LOCAL_SRC_FILES := mirror.c jdatasrc.c
LOCAL_C_INCLUDES := external/jpeg

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := libjnimirror
LOCAL_SRC_FILES := mirror_jni.c

LOCAL_STATIC_LIBRARIES := libmirror
LOCAL_SHARED_LIBRARIES := libjpeg

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_PREBUILT_LIBS := lib/libmirror.a
#include $(BUILD_MULTI_PREBUILT)
