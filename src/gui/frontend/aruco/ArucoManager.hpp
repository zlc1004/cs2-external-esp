#pragma once

#include <vector>
#include <string>
#include "../../../external/imgui/imgui.h"
#include "../../../external/AsyncLogger/include/AsyncLogger/Logger.hpp"

class ArucoManager {
public:
    ~ArucoManager() = default;
    ArucoManager(const ArucoManager&) = delete;
    ArucoManager(ArucoManager&&) = delete;
    ArucoManager& operator=(const ArucoManager&) = delete;
    ArucoManager& operator=(ArucoManager&&) = delete;

    static bool Init() {
        return GetInstance().InitImpl();
    }
    
    static void Destroy() {
        return GetInstance().DestroyImpl();
    }
    
    static ImTextureID GetMarker(int index) {
        auto& instance = GetInstance();
        if (index < 0 || index >= static_cast<int>(instance.markers.size())) {
            return 0;
        }
        return instance.markers[index];
    }
    
    static int GetMarkerCount() {
        return MARKER_COUNT;
    }

private:
    ArucoManager() = default;

    static ArucoManager& GetInstance() {
        static ArucoManager instance;
        return instance;
    }

    bool InitImpl() {
        markers.reserve(MARKER_COUNT);
        
        // For now, just push placeholder values
        // On Windows, this would actually load the PNG files
        for (int i = 0; i < MARKER_COUNT; ++i) {
            markers.push_back(0); // Placeholder texture ID
        }
        
        LOGF(al::INFO, "ArUco manager initialized with {} markers", MARKER_COUNT);
        return true;
    }
    
    void DestroyImpl() {
        markers.clear();
    }

    std::vector<ImTextureID> markers;
    static constexpr int MARKER_COUNT = 13;
};
