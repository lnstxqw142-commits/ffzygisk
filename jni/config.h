#ifndef CONFIG_H
#define CONFIG_H
struct Vec3 { float x, y, z; };
enum AimMode { AIM_OFF=-1, AIM_BODY=0, AIM_NECK=1, AIM_HEAD=2 };
struct FFSettings {
    volatile int aim_on;
    volatile int aim_mode;
    volatile int esp_on;
    volatile int esp_count_on;
    volatile int antiban_on;
    volatile int godmode_on;
    volatile int wallhack_on;
    volatile float pov_fov;
    volatile float icon_x;
    volatile float icon_y;
    volatile float menu_scale;
    volatile int menu_open;
};
static FFSettings g_cfg = { 1,2,1,1,1,0,0, 90.0f, 40.0f, 200.0f, 1.0f, 0 };
#endif
