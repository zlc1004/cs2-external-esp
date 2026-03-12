#include "Bhop.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Offsets.hpp"
#include "config/Current.hpp"
#include <Windows.h>
#include <thread>

// FL_ONGROUND flag value
constexpr uint32_t FL_ONGROUND = (1 << 0);

void Bhop::Run() {
    static int debug_count = 0;
    
    if (!cfg::bhop::enabled)
        return;

    // Check if forceJump offset is valid
    if (offsets::forceJump == 0) {
        if (debug_count < 3) {
            LOGF(WARNING, "[Bhop] ForceJump offset is 0 - bhop disabled");
            debug_count++;
        }
        return;
    }

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
    
    static bool logged_first_press = false;
    if (space_pressed && !logged_first_press) {
        LOGF(INFO, "[Bhop] Space pressed - ForceJump address: 0x{:X}", client.base + offsets::forceJump);
        logged_first_press = true;
    }
    
    if (!space_pressed) {
        // Reset force jump when space is released
        p->write<int>(client.base + offsets::forceJump, 0);
        was_in_air = false;
        return;
    }

    // Check if player is in air
    bool is_in_air = IsPlayerInAir(local_player);
    
    static int air_log_count = 0;
    if (space_pressed && air_log_count < 10) {
        LOGF(INFO, "[Bhop] In air: {} | Writing: {}", is_in_air, is_in_air ? 65537 : 256);
        air_log_count++;
    }
    
    // Simple logic: if on ground, jump. If in air, force jump on next frame
    if (!is_in_air) {
        // On ground - trigger jump
        p->write<int>(client.base + offsets::forceJump, 256);
    } else {
        // In air - prepare for next jump
        p->write<int>(client.base + offsets::forceJump, 65537);
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
