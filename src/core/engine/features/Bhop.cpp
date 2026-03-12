#include "Bhop.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Offsets.hpp"
#include "config/Current.hpp"
#include <Windows.h>
#include <thread>

// FL_ONGROUND flag value
constexpr uint32_t FL_ONGROUND = (1 << 0);

void Bhop::Run() {
    if (!cfg::bhop::enabled)
        return;

    // Check if forceJump offset is valid
    if (offsets::forceJump == 0)
        return;

    auto cache = Cache::CopySnapshot();
    auto p = Engine::GetProcess();
    auto client = Engine::GetClient();

    if (!p || client.base == 0)
        return;

    // Find local player
    Player* local_player = nullptr;
    for (auto& player : cache.players) {
        if (player.localplayer) {
            local_player = &player;
            break;
        }
    }

    if (!local_player || !local_player->alive)
        return;

    // Check if space is pressed
    bool space_pressed = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    
    if (!space_pressed) {
        // Reset force jump when space is released
        p->write<int>(client.base + offsets::forceJump, 0);
        was_in_air = false;
        return;
    }

    // Check if player is in air
    bool is_in_air = IsPlayerInAir(local_player);
    
    if (space_pressed && is_in_air) {
        // Player is in air and holding space - wait then force jump
        auto now = std::chrono::steady_clock::now();
        auto time_since_last_jump = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_jump_time).count();
        
        if (time_since_last_jump >= 5) {
            p->write<int>(client.base + offsets::forceJump, 65537); // Force jump
            last_jump_time = now;
            was_in_air = true;
        }
    }
    else if (space_pressed && !is_in_air) {
        // Player is on ground and pressing space - normal jump
        p->write<int>(client.base + offsets::forceJump, 256);
        was_in_air = false;
    }
    else if (!space_pressed && was_in_air) {
        // Space released after being in air
        p->write<int>(client.base + offsets::forceJump, 256);
        was_in_air = false;
    }
}

bool Bhop::IsPlayerInAir(Player* player) {
    if (!player)
        return false;

    auto p = Engine::GetProcess();
    if (!p)
        return false;

    // Read player flags
    uint32_t flags = p->read<uint32_t>(player->GetPawnAddress() + offsets::pawn::m_fFlags);
    
    // Check if FL_ONGROUND flag is NOT set (player is in air)
    return (flags & FL_ONGROUND) == 0;
}
