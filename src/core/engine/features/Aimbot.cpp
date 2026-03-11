#include "Aimbot.hpp"
#include "common.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Offsets.hpp"
#include "RecoilPatterns.hpp"
#include <algorithm>
#include <chrono>

#include "Aimbot.hpp"
#include "common.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Offsets.hpp"
#include "RecoilPatterns.hpp"
#include <algorithm>
#include <chrono>

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
        g_shot_index = 0;
        g_recoil_accumulator = { 0.0f, 0.0f };
        return;
    }

    // Check if left mouse button is pressed (VK_LBUTTON = 0x01)
    bool is_shooting = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

    // Apply pattern-based anti-recoil if enabled AND left mouse button is pressed
    // Anti-recoil works independently of aimbot enabled state
    if (cfg::aimbot::anti_recoil && is_shooting) {
        ApplyAntiRecoil(local_player);
    }
    // Standard RCS Logic (if anti_recoil is disabled, use old RCS when shooting)
    // This requires aimbot to be enabled
    else if (cfg::aimbot::enabled && cfg::aimbot::rcs && is_shooting && local_player->shots_fired > 0) {
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

    // If anti-recoil is enabled but not shooting, handle rebound
    if (cfg::aimbot::anti_recoil && !is_shooting) {
        // Trigger rebound if we were shooting before
        if (g_was_shooting) {
            LOGF(INFO, "[AntiRecoil] Stopped shooting - total shots: %d", g_shot_index);
            
            if (cfg::aimbot::auto_rebound && (g_recoil_accumulator.x != 0.0f || g_recoil_accumulator.y != 0.0f)) {
                LOGF(INFO, "[AntiRecoil] Rebounding - moving back by (%f, %f)", 
                    -g_recoil_accumulator.x, -g_recoil_accumulator.y);
                mouse_event(MOUSEEVENTF_MOVE, (long)(-g_recoil_accumulator.x), (long)(-g_recoil_accumulator.y), 0, 0);
            }
            g_shot_index = 0;
            g_recoil_accumulator = { 0.0f, 0.0f };
            g_was_shooting = false;
        }
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

WeaponID Aimbot::GetWeaponIDFromName(const std::string& weapon_name) {
    // Convert weapon name to ID
    // CS2 weapon names are like "weapon_ak47", "weapon_m4a1", etc.
    // We check the clean_weapon field which should have cleaned names
    
    if (weapon_name.find("ak47") != std::string::npos || weapon_name.find("AK-47") != std::string::npos) 
        return WeaponID::AK47;
    
    // M4A1-S has priority over M4A4 check
    if (weapon_name.find("m4a1_silencer") != std::string::npos || 
        weapon_name.find("m4a1-s") != std::string::npos || 
        weapon_name.find("M4A1-S") != std::string::npos) 
        return WeaponID::M4A1;
    
    if (weapon_name.find("m4a4") != std::string::npos || 
        weapon_name.find("M4A4") != std::string::npos)
        return WeaponID::M4A4;
    
    // Fallback for generic m4a1 (without silencer indicator)
    if (weapon_name.find("m4a1") != std::string::npos || weapon_name.find("M4A1") != std::string::npos)
        return WeaponID::M4A4;
    
    if (weapon_name.find("galil") != std::string::npos || weapon_name.find("Galil") != std::string::npos) 
        return WeaponID::GALIL;
    
    if (weapon_name.find("famas") != std::string::npos || weapon_name.find("FAMAS") != std::string::npos) 
        return WeaponID::FAMAS;
    
    if (weapon_name.find("aug") != std::string::npos || weapon_name.find("AUG") != std::string::npos) 
        return WeaponID::AUG;
    
    if (weapon_name.find("sg553") != std::string::npos || weapon_name.find("sg556") != std::string::npos || 
        weapon_name.find("SG") != std::string::npos) 
        return WeaponID::SG553;
    
    if (weapon_name.find("mp9") != std::string::npos || weapon_name.find("MP9") != std::string::npos) 
        return WeaponID::MP9;
    
    if (weapon_name.find("mac10") != std::string::npos || weapon_name.find("MAC-10") != std::string::npos) 
        return WeaponID::MAC10;
    
    if (weapon_name.find("ump") != std::string::npos || weapon_name.find("UMP") != std::string::npos) 
        return WeaponID::UMP45;
    
    if (weapon_name.find("mp7") != std::string::npos || weapon_name.find("MP7") != std::string::npos) 
        return WeaponID::MP7;
    
    if (weapon_name.find("mp5") != std::string::npos || weapon_name.find("MP5") != std::string::npos) 
        return WeaponID::MP5SD;
    
    if (weapon_name.find("bizon") != std::string::npos || weapon_name.find("Bizon") != std::string::npos || 
        weapon_name.find("PP-Bizon") != std::string::npos) 
        return WeaponID::BIZON;
    
    if (weapon_name.find("p90") != std::string::npos || weapon_name.find("P90") != std::string::npos) 
        return WeaponID::P90;
    
    if (weapon_name.find("cz") != std::string::npos || weapon_name.find("CZ") != std::string::npos) 
        return WeaponID::CZ75;
    
    if (weapon_name.find("m249") != std::string::npos || weapon_name.find("M249") != std::string::npos) 
        return WeaponID::M249;
    
    return WeaponID::UNKNOWN;
}

void Aimbot::ApplyAntiRecoil(Player* local_player) {
    if (!local_player) {
        LOGF(VERBOSE, "[AntiRecoil] No local player");
        return;
    }

    // Get current weapon ID
    WeaponID weapon_id = GetWeaponIDFromName(local_player->clean_weapon);
    
    // Debug: Log weapon detection
    static WeaponID last_logged_weapon = WeaponID::UNKNOWN;
    if (weapon_id != last_logged_weapon) {
        LOGF(INFO, "[AntiRecoil] Weapon detected: %s -> ID: %d", local_player->clean_weapon.c_str(), (int)weapon_id);
        last_logged_weapon = weapon_id;
    }
    
    // Check if weapon changed
    if (weapon_id != g_current_weapon) {
        g_current_weapon = weapon_id;
        g_shot_index = 0;
        g_recoil_accumulator = { 0.0f, 0.0f };
        LOGF(INFO, "[AntiRecoil] Weapon changed, resetting state");
    }

    // Check if we have a pattern for this weapon
    auto pattern_iter = RecoilPatterns::PatternMap.find(weapon_id);
    if (pattern_iter == RecoilPatterns::PatternMap.end()) {
        static bool logged_no_pattern = false;
        if (!logged_no_pattern) {
            LOGF(WARNING, "[AntiRecoil] No pattern for weapon: %s (ID: %d)", local_player->clean_weapon.c_str(), (int)weapon_id);
            logged_no_pattern = true;
        }
        return; // No pattern for this weapon
    }

    std::vector<RecoilPoint>* pattern = pattern_iter->second;
    if (!pattern || pattern->empty()) {
        LOGF(WARNING, "[AntiRecoil] Empty pattern for weapon ID: %d", (int)weapon_id);
        return;
    }

    // Get current time
    auto now = std::chrono::steady_clock::now();
    
    // Calculate time since last frame
    auto time_since_last_frame = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_last_click_time).count();
    
    // ALTERNATIVE METHOD: Track shots ourselves based on time and mouse state
    // Instead of relying on shots_fired, we simulate it based on weapon fire rate
    
    // First, check game's shots_fired value
    int game_shots = local_player->shots_fired;
    
    // Log both values for debugging
    static int last_log_shot = -1;
    if (g_shot_index != last_log_shot) {
        LOGF(INFO, "[AntiRecoil] Shot tracking - Game: %d, Our tracker: %d, Time since last: %lld ms", 
            game_shots, g_shot_index, time_since_last_frame);
        last_log_shot = g_shot_index;
    }
    
    // Determine fire rate based on weapon (milliseconds between shots)
    int fire_rate_ms = 100; // Default ~600 RPM
    switch (weapon_id) {
        case WeaponID::AK47: fire_rate_ms = 100; break;      // ~600 RPM
        case WeaponID::M4A4: fire_rate_ms = 90; break;       // ~666 RPM
        case WeaponID::M4A1: fire_rate_ms = 107; break;      // ~562 RPM
        case WeaponID::FAMAS: fire_rate_ms = 90; break;      // ~666 RPM
        case WeaponID::GALIL: fire_rate_ms = 109; break;     // ~550 RPM
        case WeaponID::AUG: fire_rate_ms = 92; break;        // ~652 RPM
        case WeaponID::P90: fire_rate_ms = 66; break;        // ~909 RPM
        default: fire_rate_ms = 100; break;
    }
    
    // Check if we're actively shooting (was shooting last frame)
    if (g_was_shooting) {
        // Increment shot counter based on time
        if (time_since_last_frame >= fire_rate_ms) {
            g_shot_index++;
            g_last_click_time = now;
            LOGF(VERBOSE, "[AntiRecoil] Shot fired! Index: %d", g_shot_index);
        }
    } else {
        // First shot - just started shooting
        g_shot_index = 1;
        g_last_click_time = now;
        LOGF(INFO, "[AntiRecoil] Started shooting - First shot");
    }
    
    g_was_shooting = true;
    
    // Apply pattern for current shot
    if (g_shot_index > 0 && g_shot_index <= (int)pattern->size()) {
        const RecoilPoint& point = (*pattern)[g_shot_index - 1]; // -1 because arrays are 0-indexed
        
        // Apply pattern scaling based on config
        float move_x = point.x * cfg::aimbot::pattern_scale;
        float move_y = point.y * cfg::aimbot::pattern_scale;
        
        LOGF(VERBOSE, "[AntiRecoil] Shot %d: move(%f, %f) from pattern(%d, %d) scale: %f", 
            g_shot_index, move_x, move_y, point.x, point.y, cfg::aimbot::pattern_scale);
        
        // Apply mouse movement
        if (std::abs(move_x) > 0.1f || std::abs(move_y) > 0.1f) {
            mouse_event(MOUSEEVENTF_MOVE, (long)move_x, (long)move_y, 0, 0);
            
            // Track accumulator for rebound
            g_recoil_accumulator.x += move_x;
            g_recoil_accumulator.y += move_y;
            
            LOGF(VERBOSE, "[AntiRecoil] Mouse moved! Total accumulator: (%f, %f)", 
                g_recoil_accumulator.x, g_recoil_accumulator.y);
        }
    } else if (g_shot_index > (int)pattern->size()) {
        LOGF(WARNING, "[AntiRecoil] Shot index %d exceeds pattern size %d", g_shot_index, (int)pattern->size());
    }
    
    g_last_shot_time = now;
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
