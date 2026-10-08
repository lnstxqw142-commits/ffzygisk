#include <jni.h>
#include <android/log.h>
#include <cstring>
#include <unistd.h>
#include <cstdio>
#include "zygisk.hpp"

#define TAG "FFZYG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;

static unsigned long getLibBase(const char* lib) {
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return 0;
    char line[512]; unsigned long b = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, lib)) { sscanf(line, "%lx-", &b); break; }
    }
    fclose(f);
    return b;
}

class FFZygisk : public zygisk::ModuleBase {
public:
    void onLoad(Api* a, JNIEnv* e) override { api = a; env = e; }
    void preAppSpecialize(AppSpecializeArgs* args) override {
        const char* p = nullptr;
        if (args && args->nice_name)
            p = env->GetStringUTFChars(args->nice_name, nullptr);
        if (p) {
            if (!strcmp(p, "com.dts.freefiremax") || !strcmp(p, "com.dts.freefireth")) {
                isFF = true;
                LOGI("Target: %s", p);
            }
            env->ReleaseStringUTFChars(args->nice_name, p);
        }
    }
    void postAppSpecialize(const AppSpecializeArgs* args) override {
        if (!isFF) return;
        LOGI("postAppSpecialize FF");
        for (int i = 0; i < 150; i++) {
            unsigned long b = getLibBase("libil2cpp.so");
            if (b) { LOGI("libil2cpp base = 0x%lx", b); break; }
            usleep(200000);
        }
        LOGI("MINIMAL MODULE LOADED OK");
    }
private:
    Api* api = nullptr; JNIEnv* env = nullptr; bool isFF = false;
};

REGISTER_ZYGISK_MODULE(FFZygisk)
