#pragma once

#include "EventDispatcher.h"
#include <cstdint>

class PatchGroup;

extern bool* map_reset;
extern bool* game_close;
extern std::uint32_t* map_type;
extern char* map_name;
extern PatchGroup* initial_load; // hacky, like everything else
void Test();

extern EventDispatcher* dispatcher;