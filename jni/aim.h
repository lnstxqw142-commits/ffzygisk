#ifndef AIM_H
#define AIM_H
#include "config.h"
void aimSetTarget(const Vec3& my, const Vec3& e, int mode, Vec3& out);
void aimLookAt(const Vec3& my, const Vec3& t, float* q);
#endif
