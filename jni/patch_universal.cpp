#include <jni.h>
#include <android/log.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <pthread.h>
#include <cmath>
#include "gfx_loader.h"
#include "offset.h"
#include "config.h"
#include "overlay.h"
#include "esp.h"
#include "aim.h"
#include "game_state.h"
#include "camera_hook.h"
#include "touch_reader.h"
#include "hook/And64InlineHook.hpp"
#include "zygisk.hpp"

#define TAG "FFZYG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;

// ============================================================
// GLOBAL FLAG - xác định mode
// ============================================================
static volatile int g_isZygiskMode = 0;
static volatile int g_applied = 0;
unsigned long g_base = 0;
static void* g_localPlayer = nullptr;

// ============================================================
// HELPERS
// ============================================================
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

// ============================================================
// HOOKS
// ============================================================
typedef int (*get_Health_t)(void*);
static get_Health_t orig_get_Health = nullptr;
static int my_get_Health(void* self) {
    if (self) {
        stateAddPlayer(self);
        if (!g_localPlayer) g_localPlayer = self;
    }
    if (g_cfg.godmode_on && self == g_localPlayer) return 999;
    return orig_get_Health ? orig_get_Health(self) : 100;
}

typedef void* (*get_Position_t)(void*);
static get_Position_t orig_get_Position = nullptr;
static void* my_get_Position(void* self) {
    return orig_get_Position ? orig_get_Position(self) : nullptr;
}

typedef void* (*get_HeadBone_t)(void*);
static get_HeadBone_t orig_get_HeadBone = nullptr;
static void* my_get_HeadBone(void* self) {
    return orig_get_HeadBone ? orig_get_HeadBone(self) : nullptr;
}

typedef void (*TakeDamage_t)(void*, void*, void*, void*, unsigned);
static TakeDamage_t orig_TakeDamage = nullptr;
static void my_TakeDamage(void* self, void* d, void* w, void* cp, unsigned vid) {
    if (g_cfg.godmode_on && self == g_localPlayer) return;
    if (orig_TakeDamage) orig_TakeDamage(self, d, w, cp, vid);
}

typedef void (*UpdateAimRotation_t)(void*);
static UpdateAimRotation_t orig_UpdateAimRotation = nullptr;
static void my_UpdateAimRotation(void* self) {
    if (orig_UpdateAimRotation) orig_UpdateAimRotation(self);
    if (!g_cfg.aim_on) return;
    if (!orig_get_Position) return;

    void* myPosPtr = orig_get_Position(self);
    if (!myPosPtr) return;
    Vec3 myPos;
    memcpy(&myPos, myPosPtr, sizeof(Vec3));

    auto enemies = stateGetEnemies();
    if (enemies.empty()) return;

    void* target = nullptr;
    float bestD = 1e9f;
    for (auto& e : enemies) {
        void* ePosPtr = orig_get_Position(e.obj);
        if (!ePosPtr) continue;
        Vec3 ep;
        memcpy(&ep, ePosPtr, sizeof(Vec3));
        float dx = ep.x - myPos.x, dy = ep.y - myPos.y, dz = ep.z - myPos.z;
        float d = sqrtf(dx*dx + dy*dy + dz*dz);
        if (d < bestD) { bestD = d; target = e.obj; }
    }
    if (!target) return;

    void* tPosPtr = orig_get_Position(target);
    if (!tPosPtr) return;
    Vec3 tp;
    memcpy(&tp, tPosPtr, sizeof(Vec3));

    Vec3 aimPos;
    aimSetTarget(myPos, tp, g_cfg.aim_mode, aimPos);

    if (OFF_SET_AIMROT) {
        float q[4];
        aimLookAt(myPos, aimPos, q);
        typedef void (*SetAim_t)(void*, float*, bool);
        SetAim_t setAim = (SetAim_t)(g_base + OFF_SET_AIMROT);
        if (setAim) setAim(self, q, true);
    }
}

typedef bool (*SendMsg_t)(void*, int, unsigned, void*, unsigned char, bool);
static SendMsg_t orig_SendMsg = nullptr;
static bool my_SendMsg(void* self, int cmd, unsigned sub,
                        void* msg, unsigned char region, bool ignore) {
    if (g_cfg.antiban_on && cmd >= 500 && cmd <= 600) {
        LOGI("ANTIBAN: block cmd=%d", cmd);
        return true;
    }
    return orig_SendMsg ? orig_SendMsg(self, cmd, sub, msg, region, ignore) : false;
}

typedef int (*cam_pw_t)(void*);
static cam_pw_t orig_cam_pw = nullptr;
static int my_cam_pw(void* self) {
    if (self) espSetCamera(self);
    return orig_cam_pw ? orig_cam_pw(self) : 1080;
}

typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;
static EGLBoolean my_eglSwapBuffers(EGLDisplay d, EGLSurface s) {
    static bool init = false;
    if (!init) { overlayInit(); init = true; }
    EGLint w = 0, h = 0;
    g_eglQuerySurface(d, s, EGL_WIDTH, &w);
    g_eglQuerySurface(d, s, EGL_HEIGHT, &h);
    if (w > 0 && h > 0 && orig_eglSwapBuffers) {
        GLint lp, lt, lb, lv[4];
        g_glGetIntegerv(GL_CURRENT_PROGRAM, &lp);
        g_glGetIntegerv(GL_TEXTURE_BINDING_2D, &lt);
        g_glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lb);
        g_glGetIntegerv(GL_VIEWPORT, lv);
        overlayRender(w, h);
        g_glUseProgram(lp);
        g_glBindTexture(GL_TEXTURE_2D, lt);
        g_glBindBuffer(GL_ARRAY_BUFFER, lb);
        g_glViewport(lv[0], lv[1], lv[2], lv[3]);
    }
    return orig_eglSwapBuffers(d, s);
}

// ============================================================
// THREAD QUÉT ENEMY
// ============================================================
static void* enemyScanThread(void*) {
    while (1) {
        auto e = stateGetEnemies();
        overlaySetEnemyCount((int)e.size());
        espReset();
        for (auto& p : e) {
            espAddEnemyFromWorld(p.pos, 10.0f, p.hp);
        }
        usleep(100000);
    }
    return nullptr;
}

// ============================================================
// APPLY ALL
// ============================================================
static void applyAll() {
    if (g_applied) return;
    g_applied = 1;

    g_base = getLibBase("libil2cpp.so");
    if (!g_base) { LOGE("libil2cpp NOT FOUND"); return; }
    LOGI("base=0x%lx (mode=%s)", g_base, g_isZygiskMode ? "ZYGISK" : "INJECT");

    gfxLoad();

    void* h = dlopen("libEGL.so", RTLD_NOW);
    if (h) {
        void* fn = dlsym(h, "eglSwapBuffers");
        if (fn) {
            A64HookFunction(fn, (void*)my_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
            LOGI("eglSwapBuffers hooked");
        }
    }

    A64HookFunction((void*)(g_base + OFF_GET_HEALTH),
                    (void*)my_get_Health, (void**)&orig_get_Health);
    A64HookFunction((void*)(g_base + OFF_TAKE_DAMAGE_PLAYER),
                    (void*)my_TakeDamage, (void**)&orig_TakeDamage);
    A64HookFunction((void*)(g_base + OFF_GET_POSITION),
                    (void*)my_get_Position, (void**)&orig_get_Position);
    A64HookFunction((void*)(g_base + OFF_GET_HEADBONE),
                    (void*)my_get_HeadBone, (void**)&orig_get_HeadBone);
    A64HookFunction((void*)(g_base + OFF_UPDATE_AIMROT),
                    (void*)my_UpdateAimRotation, (void**)&orig_UpdateAimRotation);
    A64HookFunction((void*)(g_base + OFF_SEND_MSG_LOBBY),
                    (void*)my_SendMsg, (void**)&orig_SendMsg);
    A64HookFunction((void*)(g_base + OFF_CAM_PIXEL_WIDTH),
                    (void*)my_cam_pw, (void**)&orig_cam_pw);

    cameraHookInstall(g_base);
    touchReaderSetCallback((TouchCallback)overlayTouch);
    touchReaderStart();

    pthread_t t;
    pthread_create(&t, nullptr, enemyScanThread, nullptr);
    pthread_detach(t);

    LOGI("ALL HOOKS INSTALLED");
}

// ============================================================
// MODE 1: ZYGISK
// ============================================================
class FFZygisk : public zygisk::ModuleBase {
public:
    void onLoad(Api* a, JNIEnv* e) override {
        // Set flag -> constructor biết đang ở mode Zygisk
        g_isZygiskMode = 1;
        api = a; env = e;
        LOGI("Zygisk mode");
    }
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
        for (int i = 0; i < 150; i++) {
            if (getLibBase("libil2cpp.so")) break;
            usleep(200000);
        }
        applyAll();
    }
private:
    Api* api = nullptr; JNIEnv* env = nullptr; bool isFF = false;
};

REGISTER_ZYGISK_MODULE(FFZygisk)

// ============================================================
// MODE 2: INJECT (constructor - chạy khi dlopen)
// ============================================================
__attribute__((constructor))
static void onLibraryLoad() {
    LOGI("Constructor called - waiting to detect mode");

    // Đợi 5s để biết Zygisk có set flag không
    for (int i = 0; i < 50; i++) {
        if (g_isZygiskMode) {
            LOGI("Mode = ZYGISK (skip constructor)");
            return;
        }
        usleep(100000); // 100ms x 50 = 5s
    }

    // Sau 5s không có flag -> mode INJECT
    LOGI("Mode = INJECT (start from constructor)");

    pthread_t t;
    pthread_create(&t, nullptr, [](void*) -> void* {
        // Đợi FF init xong
        sleep(3);
        applyAll();
        return nullptr;
    }, nullptr);
    pthread_detach(t);
}
