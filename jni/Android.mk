LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := ZyGames
LOCAL_SRC_FILES := patch_inject.cpp \
                   overlay.cpp \
                   esp.cpp \
                   aim.cpp \
                   game_state.cpp \
                   touch_reader.cpp \
                   camera_hook.cpp \
                   gfx_loader.cpp \
                   hook/And64InlineHook.cpp \
                   imgui/imgui.cpp \
                   imgui/imgui_draw.cpp \
                   imgui/imgui_tables.cpp \
                   imgui/imgui_widgets.cpp \
                   imgui/imgui_impl_opengl3.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/imgui $(LOCAL_PATH)/hook $(LOCAL_PATH)/include
LOCAL_LDLIBS    := -llog -ldl -lGLESv3 -lEGL
LOCAL_CFLAGS    := -std=c++17 -O2 -fvisibility=default -Wall -Wno-writable-strings -Wno-unused-variable -Wno-unused-function
LOCAL_CPPFLAGS  := -DIMGUI_IMPL_OPENGL_ES3
include $(BUILD_SHARED_LIBRARY)
