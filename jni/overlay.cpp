#include "overlay.h"
#include "esp.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include <android/log.h>
#include <cmath>
int g_scrW=1080, g_scrH=2400;
static bool g_init=false;
static bool g_drag=false;
static float g_dx=0, g_dy=0;
static const float IW=70.0f, IH=70.0f;
void overlayInit() {
    if (g_init) return;
    if (!ImGui::GetCurrentContext()) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init("#version 300 es");
        g_init = true;
    }
}
void overlaySetEnemyCount(int n) { }
int overlayGetEnemyCount() { return 0; }
void overlayRender(int w, int h) {
    if (!g_init) return;
    g_scrW=w; g_scrH=h;
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    espDrawAll();
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    float ix=g_cfg.icon_x, iy=g_cfg.icon_y;
    dl->AddRectFilled(ImVec2(ix,iy), ImVec2(ix+IW,iy+IH), IM_COL32(0,0,0,200), 10.0f);
    dl->AddRect(ImVec2(ix,iy), ImVec2(ix+IW,iy+IH), IM_COL32(0,255,0,255), 10.0f, 0, 2.0f);
    dl->AddText(ImVec2(ix+20,iy+24), IM_COL32(0,255,0,255), "FF");
    if (g_cfg.menu_open) {
        ImGui::SetNextWindowPos(ImVec2(ix+90,iy), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300,0), ImGuiCond_Always);
        ImGui::Begin("FF Menu", nullptr,
            ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|
            ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize);
        const char* am[] = {"OFF","BODY","NECK","HEAD"};
        int idx = g_cfg.aim_on ? (g_cfg.aim_mode+1) : 0;
        if (ImGui::Combo("AIM", &idx, am, 4)) {
            if (idx==0) g_cfg.aim_on=0;
            else { g_cfg.aim_on=1; g_cfg.aim_mode=idx-1; }
        }
        ImGui::Checkbox("ESP HITBOX", (bool*)&g_cfg.esp_on);
        ImGui::Checkbox("HP TEXT", (bool*)&g_cfg.esp_count_on);
        ImGui::Checkbox("ANTIBAN", (bool*)&g_cfg.antiban_on);
        ImGui::Checkbox("GODMODE", (bool*)&g_cfg.godmode_on);
        ImGui::Separator();
        float fov = (float)g_cfg.pov_fov;
        if (ImGui::SliderFloat("POV", &fov, 60.0f, 130.0f, "%.0f"))
            g_cfg.pov_fov = fov;
        float sc = (float)g_cfg.menu_scale;
        if (ImGui::SliderFloat("SIZE", &sc, 0.5f, 2.0f, "%.2f"))
            g_cfg.menu_scale = sc;
        ImGui::Separator();
        if (ImGui::Button("AN", ImVec2(-1,0))) g_cfg.menu_open=0;
        ImGui::End();
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void overlayTouch(int a, float x, float y) {
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(x,y);
    float ix=g_cfg.icon_x, iy=g_cfg.icon_y;
    if (a==0) {
        io.MouseDown[0]=true;
        if (x>=ix && x<=ix+IW && y>=iy && y<=iy+IH) {
            g_drag=true; g_dx=x-ix; g_dy=y-iy;
        }
    } else if (a==2) {
        if (g_drag) { g_cfg.icon_x=x-g_dx; g_cfg.icon_y=y-g_dy; }
    } else if (a==1) {
        io.MouseDown[0]=false;
        if (g_drag) {
            float dx=fabsf(x-(ix+g_dx)), dy=fabsf(y-(iy+g_dy));
            if (dx<12 && dy<12) g_cfg.menu_open = g_cfg.menu_open?0:1;
            g_drag=false;
        }
    }
}
