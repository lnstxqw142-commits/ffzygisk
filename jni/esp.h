#ifndef ESP_H
#define ESP_H

#include "config.h"

void espDrawAll();
void espAddEnemyFromWorld(const Vec3& pos, float dist, int hp);
void espReset();
bool worldToScreen(const Vec3& world, Vec3& out);
void espSetCamera(void* cam);

#endif
