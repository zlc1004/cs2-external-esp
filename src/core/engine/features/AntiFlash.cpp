#include "AntiFlash.hpp"
#include "common.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Offsets.hpp"

void AntiFlash::Run() {
    if (!cfg::antiflash::enabled)
        return;
    
    auto cache = Cache::CopySnapshot();
    
    Player* local_player = nullptr;
    for (auto& player : cache.players) {
        if (player.localplayer) {
            local_player = &player;
            break;
        }
    }

    if (!local_player || !local_player->alive)
        return;
    
    auto p = Engine::GetProcess();
    if (!p)
        return;
    
    // Try multiple potential flash offsets
    // cs2-dumper says 0x15F8, but CS2_External uses 0x1468
    const std::ptrdiff_t flash_offsets[] = {
        0x15F8, // m_flFlashDuration from cs2-dumper
        0x15EC, // m_flFlashOverlayAlpha 
        0x1468  // from CS2_External (might be outdated)
    };
    
    float zero = 0.0f;
    for (auto offset : flash_offsets) {
        p->write<float>(local_player->GetPawnAddress() + offset, zero);
    }
}
