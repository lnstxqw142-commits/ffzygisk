#include <jni.h>
#include <android/log.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <dirent.h>
#include <string>
#include <vector>
#include "zygisk.hpp"
#include "hook/And64InlineHook.hpp"

#define TAG "FFAF"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;

typedef FILE* (*fopen_t)(const char*, const char*);
static fopen_t orig_fopen = nullptr;
static FILE* my_fopen(const char* path, const char* mode) {
    if (path) {
        if (strstr(path, "frida") || strstr(path, "gum-js") ||
            strstr(path, "frida-agent") || strstr(path, "re.frida")) {
            LOGI("block fopen: %s", path);
            return nullptr;
        }
    }
    return orig_fopen(path, mode);
}

typedef char* (*strstr_t)(const char*, const char*);
static strstr_t orig_strstr = nullptr;
static char* my_strstr(const char* hay, const char* needle) {
    if (needle && (strstr(needle, "frida") ||
                   strstr(needle, "gum-js") ||
                   strstr(needle, "frida-agent"))) {
        return nullptr;
    }
    return orig_strstr(hay, needle);
}

typedef int (*connect_t)(int, const struct sockaddr*, socklen_t);
static connect_t orig_connect = nullptr;
static int my_connect(int fd, const struct sockaddr* addr, socklen_t len) {
    if (addr && addr->sa_family == AF_INET) {
        const struct sockaddr_in* in = (const struct sockaddr_in*)addr;
        int port = ntohs(in->sin_port);
        if (port == 27042 || port == 27043) {
            LOGI("block connect port %d", port);
            return -1;
        }
    }
    return orig_connect(fd, addr, len);
}

typedef int (*kill_t)(pid_t, int);
static kill_t orig_kill = nullptr;
static int my_kill(pid_t pid, int sig) {
    return 0;
}

static void installAntiFridaHooks() {
    void* libc = dlopen("libc.so", RTLD_NOW);
    if (!libc) { LOGE("libc not found"); return; }

    void* fp = dlsym(libc, "fopen");
    if (fp) {
        A64HookFunction(fp, (void*)my_fopen, (void**)&orig_fopen);
        LOGI("fopen hooked");
    }

    void* ss = dlsym(libc, "strstr");
    if (ss) {
        A64HookFunction(ss, (void*)my_strstr, (void**)&orig_strstr);
        LOGI("strstr hooked");
    }

    void* cn = dlsym(libc, "connect");
    if (cn) {
        A64HookFunction(cn, (void*)my_connect, (void**)&orig_connect);
        LOGI("connect hooked");
    }

    void* kl = dlsym(libc, "kill");
    if (kl) {
        A64HookFunction(kl, (void*)my_kill, (void**)&orig_kill);
        LOGI("kill hooked");
    }

    LOGI("Anti-frida bypass installed");
}

class FFAF : public zygisk::ModuleBase {
public:
    void onLoad(Api* a, JNIEnv* e) override { api = a; env = e; }

    void preAppSpecialize(AppSpecializeArgs* args) override {
        const char* p = nullptr;
        if (args && args->nice_name)
            p = env->GetStringUTFChars(args->nice_name, nullptr);
        if (p) {
            if (!strcmp(p, "com.dts.freefiremax") ||
                !strcmp(p, "com.dts.freefireth")) {
                isFF = true;
                LOGI("Target: %s", p);
            }
            env->ReleaseStringUTFChars(args->nice_name, p);
        }
    }

    void postAppSpecialize(const AppSpecializeArgs* args) override {
        if (!isFF) return;
        sleep(1);
        installAntiFridaHooks();
    }

private:
    Api* api = nullptr; JNIEnv* env = nullptr; bool isFF = false;
};

REGISTER_ZYGISK_MODULE(FFAF)
