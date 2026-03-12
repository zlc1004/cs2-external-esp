#include "Aimbot.hpp"
#include "common.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Offsets.hpp"

void Aimbot::Run() {
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

    // Check if left mouse button is pressed (VK_LBUTTON = 0x01)
    bool is_shooting = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

    // RCS using direct m_aimPunchAngle with configurable scale
    if (cfg::aimbot::rcs && is_shooting && local_player->shots_fired > 1) {
        Vec3_t punch = local_player->aim_punch;
        
        // Get current view angles
        float current_pitch = local_player->eye_angles.x;
        float current_yaw = local_player->eye_angles.y;
        
        // Use configurable RCS scale (default 1.0)
        float rcs_yaw = current_yaw - punch.y * cfg::aimbot::rcs_scale;
        float rcs_pitch = current_pitch - punch.x * cfg::aimbot::rcs_scale;
        
        game.SetViewAngle(rcs_pitch, rcs_yaw);
        
        g_old_punch = punch;
    } else {
        g_old_punch = { 0, 0, 0 };
    }

    // Aimbot logic - only runs if aimbot is enabled
    if (!cfg::aimbot::enabled)
        return;

    float best_fov = cfg::aimbot::fov * 20.0f; // Scale FOV for screen space (pixels)
    Vec3_t best_target_pos{};
    bool found_target = false;

    for (auto& player : cache.players) {
        if (player.localplayer || !player.alive)
            continue;

        if (cfg::aimbot::ignore_team && player.team == local_player->team)
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

    // Aim at target when 'C' key is pressed
    if (found_target && (GetAsyncKeyState('C') & 0x8000)) {
        AimAt(best_target_pos, game.view_matrix);
    }
}

void Aimbot::AimAt(Vec3_t target, view_matrix_t matrix) {
    auto cache = Cache::CopySnapshot();
    auto& game = cache.game;
    
    Player* local_player = nullptr;
    for (auto& player : cache.players) {
        if (player.localplayer) {
            local_player = &player;
            break;
        }
    }
    
    if (!local_player)
        return;
    
    // Calculate angle to target using CS2_External formula
    // Use position + view_offset (automatically accounts for crouch)
    Vec3_t local_pos = local_player->pos + local_player->view_offset;
    
    Vec3_t delta = target - local_pos;
    float distance = sqrt(delta.x * delta.x + delta.y * delta.y);
    
    // Calculate relative angles (difference from current view)
    float target_yaw = atan2f(delta.y, delta.x) * 57.295779513f;
    float target_pitch = -atan(delta.z / distance) * 57.295779513f;
    
    float yaw = target_yaw - local_player->eye_angles.y;
    float pitch = target_pitch - local_player->eye_angles.x;
    
    // Instant snap: add relative angle to current view angle
    yaw = yaw + local_player->eye_angles.y;
    pitch = pitch + local_player->eye_angles.x;
    
    game.SetViewAngle(pitch, yaw);
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
