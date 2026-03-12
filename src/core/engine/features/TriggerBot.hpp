#pragma once
#include "../cache/Cache.hpp"
#include <chrono>

class TriggerBot {
public:
    static void Run();

private:
    static bool ShouldShoot(Player* local_player);
    
    // Triggerbot state
    inline static std::chrono::steady_clock::time_point last_shot_time;
    inline static bool is_shooting = false;
};
