#include "overlay.h"
#include "esp.h"
#include "imgui/imgui.h"
#include <vector>
#include <mutex>
#include <cmath>
#include <cstring>
#include <android/log.h>

#define TAG "FFZYG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

extern int g_scrW, g_scrH;

// Base của libil2cpp (truyền từ patch.cpp)
extern unsigned long g_base;

// Offset WorldToScreenPoint (từ dump.cs)
#define OFF_WORLD_TO_SCREEN      0xa6b4998UL

// Callback Unity: Vector3 WorldToScreenPoint(Vector3 position)
// ABI: X0 = self (Camera), S0..S2 = position (x,y,z)
// Return: Vector3 trong S0..S2
typedef void (*WorldToScreen_t)(void* self, float* inPos, float* outPos);
static WorldToScreen_t g_w2s = nullptr;

// Camera instance hiện tại (được cập nhật khi hook get_projectionMatrix)
extern void* g_currentCamera;
static void* g_cam = nullptr;

void espSetCamera(void* cam) { g_cam = cam; }

static std::vector<EnemyBox> g_b;
static std::mutex g_m;

void espReset() { std::lock_guard<std::mutex> l(g_m); g_b.clear(); }

// Fallback: dùng viewProj matrix nếu không gọi được Unity API
extern const float* cameraGetViewProj();

bool worldToScreen(const Vec3& w, Vec3& out) {
    // Cách 1: dùng Unity API WorldToScreenPoint
    if (g_cam && g_base) {
        if (!g_w2s) {
            g_w2s = (WorldToScreen_t)(g_base + OFF_WORLD_TO_SCREEN);
        }
        if (g_w2s) {
            float in[3] = { w.x, w.y, w.z };
            float res[3] = { 0, 0, 0 };
            g_w2s(g_cam, in, res);
            // res = (screenX, screenY, depth)
            if (res[2] > 0.01f) {
                out.x = res[0];
                out.y = g_scrH - res[1]; // Unity origin bottom-left
                out.z = res[2];
                return true;
            }
        }
    }
    // Cách 2: fallback dùng matrix
    const float* vp = cameraGetViewProj();
    if (!vp || vp[0] == 0) return false;
    float cx = vp[0]*w.x + vp[4]*w.y + vp[8]*w.z + vp[12];
    float cy = vp[1]*w.x + vp[5]*w.y + vp[9]*w.z + vp[13];
    float cw = vp[3]*w.x + vp[7]*w.y + vp[11]*w.z + vp[15];
    if (cw < 0.01f) return false;
    float nx = cx/cw, ny = cy/cw;
    out.x = (nx + 1.0f) * 0.5f * (float)g_scrW;
    out.y = (1.0f - ny) * 0.5f * (float)g_scrH;
    out.z = cw;
    return true;
}

void espAddEnemyFromWorld(const Vec3& p, float d, int hp) {
    if (!g_cfg.esp_on) return;
    Vec3 t = p; t.y += 1.8f;
    Vec3 st, sb;
    if (!worldToScreen(t, st)) return;
    if (!worldToScreen(p, sb)) return;
    float w = 40.0f / (d * 0.1f + 1.0f);
    if (w < 15) w = 15;
    if (w > 100) w = 100;
    EnemyBox b;
    b.x1 = st.x - w/2; b.y1 = st.y;
    b.x2 = st.x + w/2; b.y2 = sb.y;
    b.hp = hp;
    std::lock_guard<std::mutex> l(g_m);
    g_b.push_back(b);
}

void espDrawAll() {
    if (!g_cfg.esp_on) return;
    if (!ImGui::GetCurrentContext()) return;
    std::lock_guard<std::mutex> l(g_m);
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    for (auto& b : g_b) {
        dl->AddRect(ImVec2(b.x1, b.y1), ImVec2(b.x2, b.y2),
                    IM_COL32(255, 0, 0, 230), 0, 0, 2.0f);
        if (b.hp > 0 && b.hp <= 100) {
            float bh = b.y2 - b.y1;
            float f = bh * (b.hp / 100.0f);
            dl->AddRectFilled(ImVec2(b.x1-6, b.y1), ImVec2(b.x1-3, b.y2),
                              IM_COL32(0,0,0,180));
            dl->AddRectFilled(ImVec2(b.x1-6, b.y2-f), ImVec2(b.x1-3, b.y2),
                              IM_COL32(0,255,0,230));
        }
        if (g_cfg.esp_count_on) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", b.hp);
            float tw = ImGui::CalcTextSize(buf).x;
            dl->AddText(ImVec2((b.x1+b.x2)/2 - tw/2, b.y1 - 20),
                        IM_COL32(255,255,0,255), buf);
        }
    }
}
