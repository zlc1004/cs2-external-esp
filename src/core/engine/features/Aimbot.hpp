#pragma once
#include "../cache/Cache.hpp"
#include "RecoilPatterns.hpp"
#include <chrono>

class Aimbot {
public:
    static void Run();

private:
    static void AimAt(Vec3_t target, view_matrix_t matrix);
    static float GetFov(Vec3_t target_pos, Vec3_t local_pos, view_matrix_t matrix);
    static void ApplyAntiRecoil(Player* local_player);
    static WeaponID GetWeaponIDFromName(const std::string& weapon_name);

    inline static Vec3_t g_old_punch = { 0, 0, 0 };
    
    // Anti-recoil state tracking
    inline static int g_shot_index = 0;
    inline static WeaponID g_current_weapon = WeaponID::UNKNOWN;
    inline static std::chrono::steady_clock::time_point g_last_shot_time;
    inline static Vec2_t g_recoil_accumulator = { 0.0f, 0.0f };
    inline static bool g_was_shooting = false;
    inline static std::chrono::steady_clock::time_point g_last_click_time;
};
