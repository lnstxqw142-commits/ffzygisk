#ifndef CAMERA_HOOK_H
#define CAMERA_HOOK_H

#include "config.h"

void cameraHookInstall(unsigned long il2cppBase);
void cameraSetViewProj(const float* m);
const float* cameraGetViewProj();

#endif
