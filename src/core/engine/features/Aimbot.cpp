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

    // Check activation based on mode
    bool should_aim = false;
    
    if (cfg::aimbot::mode == 0) {
        // Mode 0: Hold Key (C)
        should_aim = (GetAsyncKeyState('C') & 0x8000) != 0;
    } else if (cfg::aimbot::mode == 1) {
        // Mode 1: Auto on Shoot - activate when shooting
        should_aim = is_shooting;
    }
    
    if (found_target && should_aim) {
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
    
    // Calculate absolute target angles
    float target_yaw = atan2f(delta.y, delta.x) * 57.295779513f;
    float target_pitch = -atan(delta.z / distance) * 57.295779513f;
    
    // Read ACTUAL current view angles from dwViewAngles (not cached eye_angles)
    float current_pitch, current_yaw;
    if (!game.GetViewAngles(current_pitch, current_yaw)) {
        // Fallback to cached eye_angles if GetViewAngles fails
        current_pitch = local_player->eye_angles.x;
        current_yaw = local_player->eye_angles.y;
    }
    
    // Debug logging
    static int aim_log_count = 0;
    if (aim_log_count < 3) {
        LOGF(INFO, "[AIMBOT DEBUG] Delta: ({:.2f}, {:.2f}, {:.2f}) | Distance: {:.2f}", 
             delta.x, delta.y, delta.z, distance);
        LOGF(INFO, "[AIMBOT DEBUG] Target angles: Yaw={:.2f}, Pitch={:.2f}", target_yaw, target_pitch);
        LOGF(INFO, "[AIMBOT DEBUG] Current angles (from dwViewAngles): Yaw={:.2f}, Pitch={:.2f}", 
             current_yaw, current_pitch);
        aim_log_count++;
    }
    
    // Calculate angle delta from current view
    float yaw_delta = target_yaw - current_yaw;
    float pitch_delta = target_pitch - current_pitch;
    
    if (aim_log_count <= 3) {
        LOGF(INFO, "[AIMBOT DEBUG] Raw deltas: Yaw={:.2f}, Pitch={:.2f}", yaw_delta, pitch_delta);
    }
    
    // Normalize angle deltas to -180/+180 range
    while (yaw_delta > 180.0f) yaw_delta -= 360.0f;
    while (yaw_delta < -180.0f) yaw_delta += 360.0f;
    
    if (aim_log_count <= 3) {
        LOGF(INFO, "[AIMBOT DEBUG] Normalized deltas: Yaw={:.2f}, Pitch={:.2f}", yaw_delta, pitch_delta);
        LOGF(INFO, "[AIMBOT DEBUG] Smooth value: {:.2f}", cfg::aimbot::smooth);
    }
    
    // Apply smoothing to the delta
    // smooth=0.0 = instant snap, smooth=0.9 = very smooth
    yaw_delta *= (1.0f - cfg::aimbot::smooth);
    pitch_delta *= (1.0f - cfg::aimbot::smooth);
    
    if (aim_log_count <= 3) {
        LOGF(INFO, "[AIMBOT DEBUG] After smoothing: Yaw delta={:.2f}, Pitch delta={:.2f}", yaw_delta, pitch_delta);
    }
    
    // Recoil Control System (RCS) - Apply to deltas BEFORE calculating final angles
    if (cfg::aimbot::rcs && local_player->shots_fired > 1) {
        // CS2_External method: subtract punch angle to compensate for recoil
        // aim_punch.x affects pitch (vertical recoil)
        // aim_punch.y affects yaw (horizontal recoil)
        float rcs_scale = cfg::aimbot::rcs_scale;
        
        // Log aim punch values to debug
        static int rcs_log_count = 0;
        if (rcs_log_count < 5) {
            LOGF(INFO, "[RCS DEBUG] Shots: {} | Punch RAW: ({:.4f}, {:.4f}, {:.4f}) | Scale: {:.2f}", 
                 local_player->shots_fired, 
                 local_player->aim_punch.x, local_player->aim_punch.y, local_player->aim_punch.z,
                 rcs_scale);
            LOGF(INFO, "[RCS DEBUG] Applying to deltas - Yaw delta before: {:.4f}, Pitch delta before: {:.4f}",
                 yaw_delta, pitch_delta);
            rcs_log_count++;
        }
        
        // Apply recoil compensation to DELTAS
        // Note: aim punch in CS2 is typically 0.5 to 2.0 degrees per shot
        // Values above 50 indicate wrong offset
        if (abs(local_player->aim_punch.x) < 50.0f && abs(local_player->aim_punch.y) < 50.0f) {
            yaw_delta -= local_player->aim_punch.y * rcs_scale;
            pitch_delta -= local_player->aim_punch.x * rcs_scale;
        } else {
            LOGF(WARNING, "[RCS] Aim punch values too large - offset might be wrong! Punch: ({:.2f}, {:.2f})",
                 local_player->aim_punch.x, local_player->aim_punch.y);
        }
    }
    
    // Calculate final angles AFTER applying RCS to deltas
    float yaw = local_player->eye_angles.y + yaw_delta;
    float pitch = local_player->eye_angles.x + pitch_delta;
    
    // Normalize final angles
    // Clamp pitch to valid range (-89 to +89 degrees)
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    
    // Wrap yaw to -180/+180 range
    while (yaw > 180.0f) yaw -= 360.0f;
    while (yaw < -180.0f) yaw += 360.0f;
    
    if (aim_log_count <= 3) {
        LOGF(INFO, "[AIMBOT DEBUG] Final angles (normalized): Yaw={:.2f}, Pitch={:.2f}", yaw, pitch);
    }
    
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
