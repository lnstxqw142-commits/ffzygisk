#include "touch_reader.h"
#include <android/log.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include <cstdio>
#include <dirent.h>
#include <sys/ioctl.h>

#define TAG "FFZYG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

static volatile bool g_running = false;
static TouchCallback g_cb = nullptr;
static int g_fd = -1;

static int   g_trackId = -1;
static float g_x = 0, g_y = 0;
static float g_prevX = 0, g_prevY = 0;
static bool  g_down = false;

// Scale raw touch -> screen
static float g_rawMaxX = 1079.0f;
static float g_rawMaxY = 2399.0f;
extern int g_scrW, g_scrH;

static int findTouchDevice() {
    DIR* d = opendir("/dev/input");
    if (!d) return -1;
    struct dirent* e;
    while ((e = readdir(d)) != NULL) {
        if (strncmp(e->d_name, "event", 5) != 0) continue;
        char path[128];
        snprintf(path, sizeof(path), "/dev/input/%s", e->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;

        unsigned long absBits[((ABS_MAX + 1) + 63) / 64] = {0};
        ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits);
        int hx = (absBits[ABS_MT_POSITION_X / 64] >> (ABS_MT_POSITION_X % 64)) & 1;
        int hy = (absBits[ABS_MT_POSITION_Y / 64] >> (ABS_MT_POSITION_Y % 64)) & 1;
        if (hx && hy) {
            // Đọc kích thước max
            struct input_absinfo info;
            if (ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &info) == 0) {
                if (info.maximum > 0) g_rawMaxX = (float)info.maximum;
            }
            if (ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &info) == 0) {
                if (info.maximum > 0) g_rawMaxY = (float)info.maximum;
            }
            LOGI("Touch device: %s (raw %fx%f)", path, g_rawMaxX, g_rawMaxY);
            closedir(d);
            return fd;
        }
        close(fd);
    }
    closedir(d);
    return -1;
}

static float scaleX(float raw) {
    if (g_rawMaxX <= 0) return raw;
    return raw * ((float)g_scrW / g_rawMaxX);
}
static float scaleY(float raw) {
    if (g_rawMaxY <= 0) return raw;
    return raw * ((float)g_scrH / g_rawMaxY);
}

static void* touchThread(void*) {
    struct input_event ev;
    while (g_running) {
        if (g_fd < 0) {
            usleep(500000);
            g_fd = findTouchDevice();
            continue;
        }
        ssize_t n = read(g_fd, &ev, sizeof(ev));
        if (n != sizeof(ev)) { usleep(5000); continue; }

        if (ev.type == EV_ABS) {
            if (ev.code == ABS_MT_TRACKING_ID) {
                if (ev.value >= 0) {
                    g_trackId = ev.value;
                    g_down = true;
                    if (g_cb) g_cb(0, scaleX(g_x), scaleY(g_y));
                } else {
                    g_trackId = -1;
                    g_down = false;
                    if (g_cb) g_cb(1, scaleX(g_x), scaleY(g_y));
                }
            }
            else if (ev.code == ABS_MT_POSITION_X) {
                g_prevX = g_x;
                g_x = (float)ev.value;
            }
            else if (ev.code == ABS_MT_POSITION_Y) {
                g_prevY = g_y;
                g_y = (float)ev.value;
            }
        }
        else if (ev.type == EV_SYN) {
            if (g_down && (g_x != g_prevX || g_y != g_prevY)) {
                if (g_cb) g_cb(2, scaleX(g_x), scaleY(g_y));
            }
        }
    }
    return nullptr;
}

void touchReaderSetCallback(TouchCallback cb) { g_cb = cb; }

void touchReaderStart() {
    if (g_running) return;
    g_running = true;
    g_fd = findTouchDevice();
    pthread_t t;
    pthread_create(&t, nullptr, touchThread, nullptr);
    pthread_detach(t);
    LOGI("Touch reader started (fd=%d)", g_fd);
}

void touchReaderStop() {
    g_running = false;
    if (g_fd >= 0) { close(g_fd); g_fd = -1; }
}
