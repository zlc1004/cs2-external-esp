#pragma once

#include "common.hpp"
#include "core/engine/cache/Cache.hpp"
#include <chrono>

class Bhop {
private:
    static inline std::chrono::steady_clock::time_point last_jump_time{};
    static inline bool was_in_air = false;

public:
    static void Run();
    static bool IsPlayerInAir(Player* player);
};
