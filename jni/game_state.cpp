#include "game_state.h"
static std::vector<PlayerEntry> g_p;
static std::mutex g_m;
void stateAddPlayer(void* o) {
    std::lock_guard<std::mutex> l(g_m);
    for (auto& x : g_p) if (x.obj == o) return;
    PlayerEntry e = { o, {0,0,0}, 100, 0, false };
    g_p.push_back(e);
}
void stateClear() { std::lock_guard<std::mutex> l(g_m); g_p.clear(); }
std::vector<PlayerEntry> stateGetAll() {
    std::lock_guard<std::mutex> l(g_m); return g_p;
}
std::vector<PlayerEntry> stateGetEnemies() {
    std::lock_guard<std::mutex> l(g_m);
    std::vector<PlayerEntry> r;
    for (auto& x : g_p) if (!x.isLocal) r.push_back(x);
    return r;
}
