#!/data/data/com.termux/files/usr/bin/bash
set -e
cd ~/zygisk-ff/jni

clang++ -shared -fPIC -O2 -std=c++17 \
    -DIMGUI_IMPL_OPENGL_ES3 \
    -Wno-writable-strings -Wno-unused-variable -Wno-unused-function \
    -o ~/zygisk-ff/build/libffzygisk.so \
    patch.cpp overlay.cpp esp.cpp aim.cpp game_state.cpp \
    touch_reader.cpp camera_hook.cpp \
    hook/And64InlineHook.cpp \
    imgui/imgui.cpp imgui/imgui_draw.cpp \
    imgui/imgui_tables.cpp imgui/imgui_widgets.cpp \
    imgui/imgui_impl_opengl3.cpp \
    -llog -ldl -lGLESv3 -lEGL \
    -I. -Iimgui -Ihook -Iinclude

cd ~/zygisk-ff
cp build/libffzygisk.so module/zygisk/arm64-v8a.so
cd module
rm -f ../FFZygisk.zip
zip -r ../FFZygisk.zip . > /dev/null
cd ..
cp FFZygisk.zip ~/storage/shared/Download/

echo ""
echo "=========================================="
echo " HOAN TAT"
echo " File: ~/storage/shared/Download/FFZygisk.zip"
echo "=========================================="
ls -la FFZygisk.zip
