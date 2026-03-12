#include "Dumper.hpp"

#include "core/engine/Engine.hpp"
#include "updater/http/HttpHelper.hpp"

bool Dumper::Init() {
    return GetInstance().InitImpl();
}

bool Dumper::FetchOffsetsFromCS2Dumper() {
    return GetInstance().FetchOffsetsFromCS2DumperImpl();
}

bool Dumper::InitImpl() {
    auto process = Engine::GetProcess();
    auto client = Engine::GetClient();
    auto engine = Engine::GetEngine();

    DWORD64 temp = 0;

    // client.dll

    // View Matrix
    if (!(temp = Scan(offsets::signatures::viewMatrix, client))) {
        LOGF(FATAL, "Could not find offset for 'viewMatrix'");
        return false;
    }

    offsets::viewMatrix = temp - client.base;
    LOGF(VERBOSE, "Found 'viewMatrix' offset at 0x{:X}", offsets::viewMatrix);

    // Global Variables
    if (!(temp = Scan(offsets::signatures::globalVars, client))) {
        LOGF(FATAL, "Could not find offset for 'globalVars'");
        return false;
    }

    offsets::globalVars = temp - client.base;
    LOGF(VERBOSE, "Found 'globalVars' offset at 0x{:X}", offsets::globalVars);

    // Entity List
    if (!(temp = Scan(offsets::signatures::entityList, client))) {
        LOGF(FATAL, "Could not find offset for 'entityList'");
        return false;
    }

    offsets::entityList = temp - client.base;
    LOGF(VERBOSE, "Found 'entityList' offset at 0x{:X}", offsets::entityList);

    // Local Player Controller
    if (!(temp = Scan(offsets::signatures::localPlayerController, client))) {
        LOGF(FATAL, "Could not find offset for 'localPlayerController'");
        return false;
    }

    offsets::localPlayerController = temp - client.base;
    LOGF(VERBOSE, "Found 'localPlayerController' offset at 0x{:X}", offsets::localPlayerController);

    // C4
    if (!(temp = Scan(offsets::signatures::plantedC4, client))) {
        LOGF(FATAL, "Could not find offset for 'weaponC4'");
        return false;
    }

    offsets::plantedC4 = temp - client.base;
    LOGF(VERBOSE, "Found 'weaponC4' offset at 0x{:X}", offsets::plantedC4);

    // View Angles - Try direct offset from cs2-dumper
    // Build 14139: dwViewAngles = 36804168 decimal = 0x2319648 hex (FIXED!)
    const uintptr_t dwViewAngles_offset = 0x2319648; 
    
    // METHOD 1: Try direct offset (cs2-dumper gives absolute offset in client.dll)
    offsets::viewAngles = dwViewAngles_offset;
    LOGF(INFO, "[Method 1] Using direct ViewAngles offset: 0x{:X}", offsets::viewAngles);
    
    // METHOD 2: Try pointer dereference method (CS2_External style) as fallback
    DWORD64 viewangle_ptr = process->read<DWORD64>(client.base + dwViewAngles_offset);
    LOGF(INFO, "[Method 2] ViewAngles pointer read result: 0x{:X}", viewangle_ptr);
    
    if (viewangle_ptr != 0) {
        uintptr_t calculated_offset = (viewangle_ptr + 0x6140) - client.base;
        LOGF(INFO, "[Method 2] Calculated ViewAngles RVA: 0x{:X}", calculated_offset);
        // Use pointer method if it gives a reasonable value
        if (calculated_offset > 0 && calculated_offset < 0x10000000) {
            offsets::viewAngles = calculated_offset;
            LOGF(INFO, "[Method 2] Using pointer-based offset");
        }
    }

    // Local Player Pawn
    offsets::localPlayerPawn = 0x2067B60;
    LOGF(VERBOSE, "Using 'localPlayerPawn' offset: 0x{:X}", offsets::localPlayerPawn);

    // ForceJump - For bunnyhopping
    if (!(temp = Scan(offsets::signatures::forceJump, client))) {
        LOGF(WARNING, "Could not find offset for 'forceJump' - Bhop will be disabled");
        offsets::forceJump = 0;
    } else {
        offsets::forceJump = temp - client.base + 0x30;
        LOGF(VERBOSE, "Found 'forceJump' offset at 0x{:X}", offsets::forceJump);
    }

    // engine2.dll

    // Build Number
    if (!(temp = Scan(offsets::signatures::buildNumber, engine))) {
        LOGF(FATAL, "Could not find offset for 'buildNumber'");
        return false;
    }

    offsets::buildNumber = temp - engine.base;
    LOGF(VERBOSE, "Found 'buildNumber' offset at 0x{:X}", offsets::buildNumber);

    LOGF(INFO, "Successfully dumped offsets...");

    return true;
}

DWORD64 Dumper::Scan(const std::string sig, ProcessModule module) {
    auto process = Engine::GetProcess();

    if (!process)
        return 0;

    DWORD offsets = 0;
    DWORD64 address = 0;
    std::vector<DWORD64> list;

    //list = process->FindSignature(module, sig.data());
    list = ScanMemory(sig, module.base, module.base + 0x4000000);

    if (!list.size())
        return 0;

    if (!process->read_raw(list.at(0) + 3, &offsets, sizeof(DWORD)))
        return 0;

    address = list.at(0) + offsets + 7;
    return address;
}

std::vector<WORD> Dumper::StrSigToArray(const std::string& sig) {
    std::istringstream iss(sig);
    std::vector<WORD> bytes;
    std::string byte_str;

    while (iss >> byte_str) {
        if (byte_str == "??" || byte_str == "?")
            bytes.push_back(256);
        else
            bytes.push_back(static_cast<WORD>(std::stoul(byte_str, nullptr, 16)));
    }
    return bytes;
}

void Dumper::GetNextArray(std::vector<short>& next, const std::vector<WORD>& signature)
{
    auto size = signature.size();
    for (int i = 0; i < size; i++)
        next[signature[i]] = i;
}

void Dumper::ScanBlock(byte* buffer, const std::vector<short>& next, const std::vector<WORD>& signature, DWORD64 start, DWORD size, std::vector<DWORD64>& result)
{
    auto process = Engine::GetProcess();

    if (!process->read_raw(start, buffer, size))
        return;

    int length = signature.size();

    for (int i = 0, j, k; i < size;)
    {
        j = i; k = 0;

        for (; k < length && j < size && (signature[k] == buffer[j] || signature[k] == 256); k++, j++);

        if (k == length)
            result.push_back(start + i);

        if ((i + length) >= size)
            return;

        int Num = next[buffer[i + length]];
        if (Num == -1)
            i += (length - next[256]);
        else
            i += (length - Num);
    }
}

std::vector<DWORD64> Dumper::ScanMemory(const std::string& sig, DWORD64 start, DWORD64 end, int number)
{
    std::vector<DWORD64> result;
    std::vector<short> next(260, -1);

    auto process = Engine::GetProcess();

    if (!process)
        return result;

    byte* buffer = new byte[MAX_BLOCK_SIZE];

    auto signature = StrSigToArray(sig);
    if (!signature.size())
        return result;

    GetNextArray(next, signature);

    MEMORY_BASIC_INFORMATION mbi;
    while (VirtualQueryEx(process->handle_, reinterpret_cast<LPCVOID>(start), &mbi, sizeof(mbi)) != 0)
    {
        int searches = 0;
        auto size = mbi.RegionSize;

        while (size >= MAX_BLOCK_SIZE)
        {
            if (result.size() >= number) {
                delete[] buffer;
	            return result;
            }

            ScanBlock(buffer, next, signature, start + (MAX_BLOCK_SIZE * searches), MAX_BLOCK_SIZE, result);

            size -= MAX_BLOCK_SIZE;
            searches++;
        }

        ScanBlock(buffer, next, signature, start + (MAX_BLOCK_SIZE * searches), size, result);

        start += mbi.RegionSize;

        if (result.size() >= number || end != 0 && start > end)
            break;
    }

	delete[] buffer;
	return result;
}

bool Dumper::FetchOffsetsFromCS2DumperImpl() {
    LOGF(INFO, "Attempting to fetch offsets from cs2-dumper repository...");
    
    json client_json;
    
    // Fetch client_dll.json for pawn offsets
    auto http_status = HttpHelper::Get("https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/client_dll.json", client_json);
    
    if (http_status != 200) {
        LOGF(WARNING, "Failed to fetch offsets from cs2-dumper (HTTP {}), using hardcoded fallback values", http_status);
        return false;
    }
    
    try {
        // Parse pawn offsets from C_CSPlayerPawnBase class (base class has the offsets)
        auto pawn_base_fields = client_json["client.dll"]["classes"]["C_CSPlayerPawnBase"]["fields"];
        
        // Update static offsets that can change between CS2 versions
        // Note: We only update pawn offsets, module offsets are scanned dynamically
        
        auto shots_fired = pawn_base_fields.value("m_iShotsFired", 0);
        auto aim_punch = pawn_base_fields.value("m_aimPunchAngle", 0);
        auto id_ent_index = pawn_base_fields.value("m_iIDEntIndex", 0);
        auto flash_duration = pawn_base_fields.value("m_flFlashDuration", 0);
        auto aim_punch_cache = pawn_base_fields.value("m_aimPunchCache", 0);
        
        LOGF(INFO, "Fetched offsets from cs2-dumper:");
        LOGF(INFO, "  m_iShotsFired: 0x{:X} (was 0x270C)", shots_fired);
        LOGF(INFO, "  m_aimPunchAngle: 0x{:X} (was 0x16CC)", aim_punch);
        LOGF(INFO, "  m_iIDEntIndex: 0x{:X} (was 0x3EAC)", id_ent_index);
        LOGF(INFO, "  m_flFlashDuration: 0x{:X} (was 0x15F8)", flash_duration);
        
        // Verify offsets are reasonable (sanity check)
        if (shots_fired == 0 || aim_punch == 0) {
            LOGF(WARNING, "Fetched offsets seem invalid, using hardcoded values");
            return false;
        }
        
        LOGF(INFO, "Successfully fetched and validated offsets from cs2-dumper");
        LOGF(WARNING, "NOTE: Auto-fetched offsets are logged but NOT applied automatically.");
        LOGF(WARNING, "      This is a safety feature. If offsets differ significantly, update Offsets.hpp manually.");
        
        return true;
    }
    catch (std::exception& e) {
        LOGF(WARNING, "Failed to parse cs2-dumper offsets: {}", e.what());
        LOGF(WARNING, "Using hardcoded fallback values");
        return false;
    }
}