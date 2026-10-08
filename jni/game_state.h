#ifndef GAME_STATE_H
#define GAME_STATE_H
#include "config.h"
#include <vector>
#include <mutex>
struct PlayerEntry { void* obj; Vec3 pos; int hp; int teamId; bool isLocal; };
void stateAddPlayer(void* obj);
void stateClear();
std::vector<PlayerEntry> stateGetAll();
std::vector<PlayerEntry> stateGetEnemies();
#endif
