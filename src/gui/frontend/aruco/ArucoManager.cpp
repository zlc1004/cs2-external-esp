#include "ArucoManager.hpp"
#include "../../../external/AsyncLogger/Logger.hpp"

bool ArucoManager::Init() {
    return GetInstance().InitImpl();
}

void ArucoManager::Destroy() {
    return GetInstance().DestroyImpl();
}

ImTextureID ArucoManager::GetMarker(int index) {
    auto& instance = GetInstance();
    if (index < 0 || index >= static_cast<int>(instance.markers.size())) {
        return 0;
    }
    return instance.markers[index];
}

int ArucoManager::GetMarkerCount() {
    return MARKER_COUNT;
}

bool ArucoManager::InitImpl() {
    markers.reserve(MARKER_COUNT);
    
    // For now, just push placeholder values
    // On Windows, this would actually load the PNG files
    for (int i = 0; i < MARKER_COUNT; ++i) {
        markers.push_back(0); // Placeholder texture ID
    }
    
    LOGF(al::INFO, "ArUco manager initialized with {} markers", MARKER_COUNT);
    return true;
}

void ArucoManager::DestroyImpl() {
    markers.clear();
}
