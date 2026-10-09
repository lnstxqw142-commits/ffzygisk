LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := ffaf
LOCAL_SRC_FILES := patch_antifrida.cpp hook/And64InlineHook.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/hook
LOCAL_LDLIBS    := -llog -ldl
LOCAL_CFLAGS    := -std=c++17 -O2 -fvisibility=default -Wall -Wno-writable-strings -Wno-unused-variable
include $(BUILD_SHARED_LIBRARY)
