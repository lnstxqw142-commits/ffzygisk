#ifndef OVERLAY_H
#define OVERLAY_H
#include "config.h"
void overlayInit();
void overlayRender(int w, int h);
void overlayTouch(int action, float x, float y);
void overlaySetEnemyCount(int n);
int  overlayGetEnemyCount();
struct EnemyBox { float x1,y1,x2,y2; int hp; };
#endif
