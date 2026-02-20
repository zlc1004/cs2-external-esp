#pragma once
#include "../cache/Cache.hpp"

class Aimbot {
public:
    static void Run();

private:
    static void AimAt(Vec3_t target, view_matrix_t matrix);
    static float GetFov(Vec3_t target_pos, Vec3_t local_pos, view_matrix_t matrix);

    inline static Vec3_t g_old_punch = { 0, 0, 0 };
};
