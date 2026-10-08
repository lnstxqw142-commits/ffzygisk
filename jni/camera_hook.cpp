#include "camera_hook.h"
#include "hook/And64InlineHook.hpp"
#include <android/log.h>
#include <cstring>
#include <cmath>

#define TAG "FFZYG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// ============================================================
// OFFSET Camera (RVA trong libil2cpp.so)
// ============================================================
#define OFF_GET_FOV              0xa6b345cUL
#define OFF_SET_FOV              0xa6b3498UL
#define OFF_GET_PROJ_MATRIX      0xa6b44f4UL
#define OFF_GET_WORLD2CAM_MATRIX 0xa6b43bcUL
#define OFF_WORLD_TO_SCREEN      0xa6b4998UL

// ============================================================
// LƯU MATRIX
// ============================================================
static float g_viewProj[16] = {0};
static float g_lastProj[16] = {0};
static float g_lastView[16] = {0};

void cameraSetViewProj(const float* m) {
    memcpy(g_viewProj, m, sizeof(g_viewProj));
}

const float* cameraGetViewProj() {
    return g_viewProj;
}

// ============================================================
// HOOK get_fieldOfView -> trả về POV setting
// ============================================================
typedef float (*get_FOV_t)(void*);
static get_FOV_t orig_get_FOV = nullptr;

static float my_get_FOV(void* self) {
    // Trả về POV custom nếu hợp lệ
    float f = g_cfg.pov_fov;
    if (f < 60.0f) f = 60.0f;
    if (f > 130.0f) f = 130.0f;
    return f;
}

// ============================================================
// HOOK set_fieldOfView -> chặn game set lại
// ============================================================
typedef void (*set_FOV_t)(void*, float);
static set_FOV_t orig_set_FOV = nullptr;

static void my_set_FOV(void* self, float v) {
    // Bỏ qua, không cho game ghi đè
    if (orig_set_FOV) orig_set_FOV(self, g_cfg.pov_fov);
}

// ============================================================
// HOOK get_projectionMatrix -> chặn + lưu
// ============================================================
typedef void (*get_Matrix_t)(void*, float*);
static get_Matrix_t orig_getProj = nullptr;
static get_Matrix_t orig_getWorld2Cam = nullptr;

static void my_getProj(void* self, float* out) {
    if (orig_getProj) orig_getProj(self, out);
    if (out) {
        memcpy(g_lastProj, out, 64);
        // Tính lại FOV nếu cần (không bắt buộc)
        // viewProj = proj * view (nhân ma trận nếu cần)
        // Nhưng để đơn giản, dùng proj trực tiếp cho worldToScreen
        memcpy(g_viewProj, out, 64);
    }
}

static void my_getWorld2Cam(void* self, float* out) {
    if (orig_getWorld2Cam) orig_getWorld2Cam(self, out);
    if (out) {
        memcpy(g_lastView, out, 64);
    }
}

// ============================================================
// HÀM NHÂN 2 MA TRẬN 4x4
// ============================================================
static void matMul4x4(const float* a, const float* b, float* out) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float s = 0;
            for (int k = 0; k < 4; k++) {
                s += a[i*4 + k] * b[k*4 + j];
            }
            out[i*4 + j] = s;
        }
    }
}

// ============================================================
// CÀI ĐẶT
// ============================================================
void cameraHookInstall(unsigned long base) {
    void* pFovGet = (void*)(base + OFF_GET_FOV);
    void* pFovSet = (void*)(base + OFF_SET_FOV);
    void* pProj   = (void*)(base + OFF_GET_PROJ_MATRIX);
    void* pW2Cam  = (void*)(base + OFF_GET_WORLD2CAM_MATRIX);

    A64HookFunction(pFovGet, (void*)my_get_FOV, (void**)&orig_get_FOV);
    LOGI("Camera.get_fieldOfView hooked");

    A64HookFunction(pFovSet, (void*)my_set_FOV, (void**)&orig_set_FOV);
    LOGI("Camera.set_fieldOfView hooked");

    A64HookFunction(pProj, (void*)my_getProj, (void**)&orig_getProj);
    LOGI("Camera.get_projectionMatrix hooked");

    A64HookFunction(pW2Cam, (void*)my_getWorld2Cam, (void**)&orig_getWorld2Cam);
    LOGI("Camera.get_worldToCameraMatrix hooked");
}
