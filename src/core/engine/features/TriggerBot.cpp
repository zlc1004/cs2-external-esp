#include "TriggerBot.hpp"
#include "common.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Offsets.hpp"
#include <Windows.h>
#include <thread>
#include <chrono>

void TriggerBot::Run() {
    auto cache = Cache::CopySnapshot();
    
    Player* local_player = nullptr;
    for (auto& player : cache.players) {
        if (player.localplayer) {
            local_player = &player;
            break;
        }
    }

    if (!local_player || !local_player->alive) {
        is_shooting = false;
        return;
    }

    // Check if triggerbot is enabled and hotkey is pressed
    if (!cfg::triggerbot::enabled) {
        return;
    }

    // Check hotkey (default: ALT key)
    bool hotkey_pressed = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
    
    static bool logged_hotkey = false;
    if (hotkey_pressed && !logged_hotkey) {
        LOGF(INFO, "[TriggerBot] Hotkey pressed, TriggerBot active");
        logged_hotkey = true;
    }
    
    if (!hotkey_pressed) {
        is_shooting = false;
        logged_hotkey = false;
        return;
    }

    // Check if we should shoot
    bool should_shoot = ShouldShoot(local_player);
    
    static int debug_count = 0;
    if (debug_count < 5) {
        LOGF(INFO, "[TriggerBot] ShouldShoot returned: {}", should_shoot);
        debug_count++;
    }
    
	if (should_shoot) {
		auto now = std::chrono::steady_clock::now();
		auto time_since_last_shot = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_shot_time).count();
		
		// Apply delay (convert to milliseconds)
		long long delay_ms = static_cast<long long>(cfg::triggerbot::delay);
		
		if (time_since_last_shot >= delay_ms) {
			// Check if player is already manually shooting
			const bool isAlreadyShooting = GetAsyncKeyState(VK_LBUTTON) < 0;
			
			if (!isAlreadyShooting) {
				// Simulate mouse click
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				
				LOGF(INFO, "[TriggerBot] Shot fired!");
			}
			
			last_shot_time = now;
			is_shooting = true;
		}
	} else {
		is_shooting = false;
	}
}

bool TriggerBot::ShouldShoot(Player* local_player) {
    if (!local_player)
        return false;
    
    auto p = Engine::GetProcess();
    auto client = Engine::GetClient();
    
    if (!p || client.base == 0)
        return false;
    
	// Read crosshair entity ID (m_iIDEntIndex)
	DWORD crosshair_id = p->read<DWORD>(local_player->GetPawnAddress() + offsets::pawn::m_iIDEntIndex);
	
	static int debug_count_id = 0;
	if (debug_count_id < 3) {
		LOGF(INFO, "[TriggerBot] Crosshair ID: {}", crosshair_id);
		debug_count_id++;
	}
	
	if (crosshair_id == 0 || crosshair_id == (DWORD)-1) {
		return false; // Not aiming at anything
	}
    
    // Calculate entity address from ID
    DWORD64 entity_list = p->read<DWORD64>(client.base + offsets::entityList);
    if (entity_list == 0)
        return false;
    
    DWORD64 list_entry = 0;
    if (!p->read_raw(entity_list + 0x8 * (crosshair_id >> 9) + 0x10, &list_entry, sizeof(DWORD64)))
        return false;
    
    if (list_entry == 0)
        return false;
    
    DWORD64 pawn_address = 0;
    if (!p->read_raw(list_entry + 0x78 * (crosshair_id & 0x1FF), &pawn_address, sizeof(DWORD64)))
        return false;
    
    if (pawn_address == 0)
        return false;
    
	// Read target health and team
	int target_health = p->read<int>(pawn_address + offsets::pawn::m_iHealth);
	uint8_t target_team = p->read<uint8_t>(pawn_address + offsets::pawn::m_iTeamNum);
	
	// Check if target is valid
	bool is_valid_target = target_health > 0;
	
	// Check team if enabled
	if (cfg::triggerbot::ignore_team) {
		is_valid_target = is_valid_target && (target_team != local_player->team);
	}
	
	static int debug_count_target = 0;
	if (is_valid_target && debug_count_target < 3) {
		LOGF(INFO, "[TriggerBot] Valid target - Health: {}, Target Team: {}, Local Team: {}", 
			target_health, target_team, local_player->team);
		debug_count_target++;
	}
	
	return is_valid_target;
}
