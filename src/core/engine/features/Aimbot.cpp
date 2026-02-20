#include "Aimbot.hpp"
#include "common.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Offsets.hpp"
#include <algorithm>

void Aimbot::Run() {
    if (!cfg::aimbot::enabled)
        return;

    auto cache = Cache::CopySnapshot();
    auto& game = cache.game;
    
    Player* local_player = nullptr;
    for (auto& player : cache.players) {
        if (player.localplayer) {
            local_player = &player;
            break;
        }
    }

    if (!local_player || !local_player->alive) {
        g_old_punch = { 0, 0, 0 };
        return;
    }

    // RCS Logic
    if (cfg::aimbot::rcs && local_player->shots_fired > 0) {
        Vec3_t punch = local_player->aim_punch;
        Vec3_t delta = punch - g_old_punch;

        // In CS2, compensation is usually 2.0
        float rcs_x = -(delta.y * 2.0f) / 0.022f;
        float rcs_y = (delta.x * 2.0f) / 0.022f;

        if (std::abs(rcs_x) > 0.1f || std::abs(rcs_y) > 0.1f) {
            mouse_event(MOUSEEVENTF_MOVE, (long)rcs_x, (long)rcs_y, 0, 0);
        }

        g_old_punch = punch;
    } else {
        g_old_punch = { 0, 0, 0 };
    }

    float best_fov = cfg::aimbot::fov * 20.0f; // Scale FOV for screen space (pixels)
    Vec3_t best_target_pos{};
    bool found_target = false;

    for (auto& player : cache.players) {
        if (player.localplayer || !player.alive || player.team == local_player->team)
            continue;

        if (cfg::aimbot::vis_check && !player.spotted)
            continue;

        Vec3_t target_pos = player.bone_list[cfg::aimbot::bone].pos;
        float fov = GetFov(target_pos, local_player->pos, game.view_matrix);

        if (fov < best_fov) {
            best_fov = fov;
            best_target_pos = target_pos;
            found_target = true;
        }
    }

    if (found_target && (GetAsyncKeyState('C') & 0x8000)) {
        AimAt(best_target_pos, game.view_matrix);
    }
}

void Aimbot::AimAt(Vec3_t target, view_matrix_t matrix) {
    Vec2_t screen_size = { (float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN) };
    
    Vec2_t screen_pos;
    if (!matrix.wts(target, screen_size, screen_pos))
        return;

    float center_x = screen_size.x / 2.0f;
    float center_y = screen_size.y / 2.0f;

    float delta_x = screen_pos.x - center_x;
    float delta_y = screen_pos.y - center_y;

    // Deadzone: If we are within 2 pixels of the target, stop moving to prevent jitter
    if (std::abs(delta_x) < 2.0f && std::abs(delta_y) < 2.0f)
        return;

    // "Bit-by-bit" movement
    float smooth = 10.0f;
    
    float move_x = delta_x / smooth;
    float move_y = delta_y / smooth;

    // Clamp the maximum movement per frame to allow faster target acquisition
    float max_move = 30.0f; 
    if (move_x > max_move) move_x = max_move;
    else if (move_x < -max_move) move_x = -max_move;

    if (move_y > max_move) move_y = max_move;
    else if (move_y < -max_move) move_y = -max_move;

    // Smallest possible move is 1 pixel if we are outside the deadzone
    if (std::abs(move_x) < 1.0f && std::abs(delta_x) >= 2.0f) move_x = (delta_x > 0) ? 1.0f : -1.0f;
    if (std::abs(move_y) < 1.0f && std::abs(delta_y) >= 2.0f) move_y = (delta_y > 0) ? 1.0f : -1.0f;

    mouse_event(MOUSEEVENTF_MOVE, (long)move_x, (long)move_y, 0, 0);
}

float Aimbot::GetFov(Vec3_t target_pos, Vec3_t local_pos, view_matrix_t matrix) {
    Vec2_t screen_size = { (float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN) };
    Vec2_t screen_pos;
    
    if (!matrix.wts(target_pos, screen_size, screen_pos))
        return 9999.0f;

    float center_x = screen_size.x / 2.0f;
    float center_y = screen_size.y / 2.0f;

    float dx = screen_pos.x - center_x;
    float dy = screen_pos.y - center_y;

    return std::sqrt(dx * dx + dy * dy);
}
