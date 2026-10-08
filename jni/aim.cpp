#include "aim.h"
#include <cmath>
#define D2R 0.0174533f
#define R2D 57.29578f
void aimSetTarget(const Vec3& my, const Vec3& e, int mode, Vec3& out) {
    float y = (mode==AIM_BODY)?0.9f:(mode==AIM_NECK)?1.4f:1.7f;
    out.x = e.x; out.y = e.y + y; out.z = e.z;
}
void aimLookAt(const Vec3& my, const Vec3& t, float* q) {
    float dx=t.x-my.x, dy=t.y-my.y, dz=t.z-my.z;
    float yaw = atan2f(dx, dz) * R2D;
    float pitch = -atan2f(dy, sqrtf(dx*dx+dz*dz)) * R2D;
    float hy=yaw*0.5f*D2R, hp=pitch*0.5f*D2R;
    float cy=cosf(hy), sy=sinf(hy), cp=cosf(hp), sp=sinf(hp);
    q[0]=cy*sp; q[1]=sy*cp; q[2]=-sy*sp; q[3]=cy*cp;
}
