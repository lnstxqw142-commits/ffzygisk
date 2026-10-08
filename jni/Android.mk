LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := ffzygisk
LOCAL_SRC_FILES := patch.cpp
LOCAL_LDLIBS    := -llog -ldl
LOCAL_CFLAGS    := -std=c++17 -O2 -fvisibility=default
include $(BUILD_SHARED_LIBRARY)
