#ifndef GFX_LOADER_H
#define GFX_LOADER_H

#include <dlfcn.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>

// Load GLES/EGL functions at runtime
// Không cần link -lGLESv3 -lEGL

void gfxLoad();
void gfxUnload();

// GL
extern PFNGLGENBUFFERSPROC            g_glGenBuffers;
extern PFNGLBINDBUFFERPROC            g_glBindBuffer;
extern PFNGLBUFFERDATAPROC            g_glBufferData;
extern PFNGLCREATESHADERPROC          g_glCreateShader;
extern PFNGLSHADERSOURCEPROC          g_glShaderSource;
extern PFNGLCOMPILESHADERPROC         g_glCompileShader;
extern PFNGLCREATEPROGRAMPROC         g_glCreateProgram;
extern PFNGLATTACHSHADERPROC          g_glAttachShader;
extern PFNGLLINKPROGRAMPROC           g_glLinkProgram;
extern PFNGLUSEPROGRAMPROC            g_glUseProgram;
extern PFNGLGETUNIFORMLOCATIONPROC    g_glGetUniformLocation;
extern PFNGLUNIFORMMATRIX4FVPROC      g_glUniformMatrix4fv;
extern PFNGLGETATTRIBLOCATIONPROC     g_glGetAttribLocation;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC g_glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC   g_glVertexAttribPointer;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC g_glDisableVertexAttribArray;
extern PFNGLDELETEBUFFERSPROC         g_glDeleteBuffers;
extern PFNGLDELETEPROGRAMPROC         g_glDeleteProgram;
extern PFNGLDELETESHADERPROC          g_glDeleteShader;
extern PFNGLGETINTEGERVPROC           g_glGetIntegerv;
extern PFNGLVIEWPORTPROC              g_glViewport;
extern PFNGLBLENDFUNCPROC             g_glBlendFunc;
extern PFNGLENABLEPROC                g_glEnable;
extern PFNGLDISABLEPROC               g_glDisable;
extern PFNGLSCISSORPROC               g_glScissor;
extern PFNGLGENTEXTURESPROC           g_glGenTextures;
extern PFNGLBINDTEXTUREPROC           g_glBindTexture;
extern PFNGLTEXIMAGE2DPROC            g_glTexImage2D;
extern PFNGLTEXPARAMETERIPROC         g_glTexParameteri;
extern PFNGLDELETETEXTURESPROC        g_glDeleteTextures;
extern PFNGLACTIVETEXTUREPROC         g_glActiveTexture;
extern PFNGLDRAWARRAYSPROC            g_glDrawArrays;
extern PFNGLDRAWELEMENTSPROC          g_glDrawElements;
extern PFNGLPIXELSTOREIPROC           g_glPixelStorei;
extern PFNGLREADPIXELSPROC            g_glReadPixels;

// EGL
extern PFNEGLGETDISPLAYPROC            g_eglGetDisplay;
extern PFNEGLINITIALIZEPROC            g_eglInitialize;
extern PFNEGLCHOOSECONFIGPROC          g_eglChooseConfig;
extern PFNEGLCREATECONTEXTPROC         g_eglCreateContext;
extern PFNEGLMAKECURRENTPROC           g_eglMakeCurrent;
extern PFNEGLSWAPBUFFERSPROC           g_eglSwapBuffers;
extern PFNEGLQUERYSURFACEPROC          g_eglQuerySurface;
extern PFNEGLGETCURRENTDISPLAYPROC     g_eglGetCurrentDisplay;
extern PFNEGLGETCURRENTSURFACEPROC     g_eglGetCurrentSurface;

#endif
