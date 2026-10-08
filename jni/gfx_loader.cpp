#include "gfx_loader.h"
#include <android/log.h>
#include <cstring>

#define TAG "FFZYG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// GL function pointers
PFNGLGENBUFFERSPROC            g_glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC            g_glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC            g_glBufferData = nullptr;
PFNGLCREATESHADERPROC          g_glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC          g_glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC         g_glCompileShader = nullptr;
PFNGLCREATEPROGRAMPROC         g_glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC          g_glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC           g_glLinkProgram = nullptr;
PFNGLUSEPROGRAMPROC            g_glUseProgram = nullptr;
PFNGLGETUNIFORMLOCATIONPROC    g_glGetUniformLocation = nullptr;
PFNGLUNIFORMMATRIX4FVPROC      g_glUniformMatrix4fv = nullptr;
PFNGLGETATTRIBLOCATIONPROC     g_glGetAttribLocation = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC g_glEnableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC   g_glVertexAttribPointer = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC g_glDisableVertexAttribArray = nullptr;
PFNGLDELETEBUFFERSPROC         g_glDeleteBuffers = nullptr;
PFNGLDELETEPROGRAMPROC         g_glDeleteProgram = nullptr;
PFNGLDELETESHADERPROC          g_glDeleteShader = nullptr;
PFNGLGETINTEGERVPROC           g_glGetIntegerv = nullptr;
PFNGLVIEWPORTPROC              g_glViewport = nullptr;
PFNGLBLENDFUNCPROC             g_glBlendFunc = nullptr;
PFNGLENABLEPROC                g_glEnable = nullptr;
PFNGLDISABLEPROC               g_glDisable = nullptr;
PFNGLSCISSORPROC               g_glScissor = nullptr;
PFNGLGENTEXTURESPROC           g_glGenTextures = nullptr;
PFNGLBINDTEXTUREPROC           g_glBindTexture = nullptr;
PFNGLTEXIMAGE2DPROC            g_glTexImage2D = nullptr;
PFNGLTEXPARAMETERIPROC         g_glTexParameteri = nullptr;
PFNGLDELETETEXTURESPROC        g_glDeleteTextures = nullptr;
PFNGLACTIVETEXTUREPROC         g_glActiveTexture = nullptr;
PFNGLDRAWARRAYSPROC            g_glDrawArrays = nullptr;
PFNGLDRAWELEMENTSPROC          g_glDrawElements = nullptr;
PFNGLPIXELSTOREIPROC           g_glPixelStorei = nullptr;
PFNGLREADPIXELSPROC            g_glReadPixels = nullptr;

// EGL function pointers
PFNEGLGETDISPLAYPROC            g_eglGetDisplay = nullptr;
PFNEGLINITIALIZEPROC            g_eglInitialize = nullptr;
PFNEGLCHOOSECONFIGPROC          g_eglChooseConfig = nullptr;
PFNEGLCREATECONTEXTPROC         g_eglCreateContext = nullptr;
PFNEGLMAKECURRENTPROC           g_eglMakeCurrent = nullptr;
PFNEGLSWAPBUFFERSPROC           g_eglSwapBuffers = nullptr;
PFNEGLQUERYSURFACEPROC          g_eglQuerySurface = nullptr;
PFNEGLGETCURRENTDISPLAYPROC     g_eglGetCurrentDisplay = nullptr;
PFNEGLGETCURRENTSURFACEPROC     g_eglGetCurrentSurface = nullptr;

static void* g_gl_handle = nullptr;
static void* g_egl_handle = nullptr;

#define L(sym) g_##sym = (decltype(g_##sym))dlsym(g_gl_handle, #sym); if (!g_##sym) LOGI("miss " #sym)

void gfxLoad() {
    if (g_gl_handle) return;

    g_gl_handle = dlopen("libGLESv3.so", RTLD_NOW);
    if (!g_gl_handle) {
        g_gl_handle = dlopen("libGLESv2.so", RTLD_NOW);
    }
    if (!g_gl_handle) {
        LOGE("Cannot load libGLESv3.so / libGLESv2.so");
        return;
    }

    g_egl_handle = dlopen("libEGL.so", RTLD_NOW);
    if (!g_egl_handle) {
        LOGE("Cannot load libEGL.so");
        return;
    }

    // GL
    L(glGenBuffers);
    L(glBindBuffer);
    L(glBufferData);
    L(glCreateShader);
    L(glShaderSource);
    L(glCompileShader);
    L(glCreateProgram);
    L(glAttachShader);
    L(glLinkProgram);
    L(glUseProgram);
    L(glGetUniformLocation);
    L(glUniformMatrix4fv);
    L(glGetAttribLocation);
    L(glEnableVertexAttribArray);
    L(glVertexAttribPointer);
    L(glDisableVertexAttribArray);
    L(glDeleteBuffers);
    L(glDeleteProgram);
    L(glDeleteShader);
    L(glGetIntegerv);
    L(glViewport);
    L(glBlendFunc);
    L(glEnable);
    L(glDisable);
    L(glScissor);
    L(glGenTextures);
    L(glBindTexture);
    L(glTexImage2D);
    L(glTexParameteri);
    L(glDeleteTextures);
    L(glActiveTexture);
    L(glDrawArrays);
    L(glDrawElements);
    L(glPixelStorei);
    L(glReadPixels);

    // EGL
    g_eglGetDisplay       = (PFNEGLGETDISPLAYPROC)       dlsym(g_egl_handle, "eglGetDisplay");
    g_eglInitialize       = (PFNEGLINITIALIZEPROC)       dlsym(g_egl_handle, "eglInitialize");
    g_eglChooseConfig     = (PFNEGLCHOOSECONFIGPROC)     dlsym(g_egl_handle, "eglChooseConfig");
    g_eglCreateContext    = (PFNEGLCREATECONTEXTPROC)    dlsym(g_egl_handle, "eglCreateContext");
    g_eglMakeCurrent      = (PFNEGLMAKECURRENTPROC)      dlsym(g_egl_handle, "eglMakeCurrent");
    g_eglSwapBuffers      = (PFNEGLSWAPBUFFERSPROC)      dlsym(g_egl_handle, "eglSwapBuffers");
    g_eglQuerySurface     = (PFNEGLQUERYSURFACEPROC)     dlsym(g_egl_handle, "eglQuerySurface");
    g_eglGetCurrentDisplay= (PFNEGLGETCURRENTDISPLAYPROC)dlsym(g_egl_handle, "eglGetCurrentDisplay");
    g_eglGetCurrentSurface= (PFNEGLGETCURRENTSURFACEPROC)dlsym(g_egl_handle, "eglGetCurrentSurface");

    LOGI("GLES/EGL loaded");
}

void gfxUnload() {
    if (g_gl_handle)  { dlclose(g_gl_handle);  g_gl_handle = nullptr; }
    if (g_egl_handle) { dlclose(g_egl_handle); g_egl_handle = nullptr; }
}
