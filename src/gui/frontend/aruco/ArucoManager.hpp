#pragma once

#include <vector>
#include <string>
#include "../../../external/imgui/imgui.h"

class ArucoManager {
public:
    ~ArucoManager() = default;
    ArucoManager(const ArucoManager&) = delete;
    ArucoManager(ArucoManager&&) = delete;
    ArucoManager& operator=(const ArucoManager&) = delete;
    ArucoManager& operator=(ArucoManager&&) = delete;

    static bool Init();
    static void Destroy();
    static ImTextureID GetMarker(int index);
    static int GetMarkerCount();

private:
    ArucoManager() = default;

    static ArucoManager& GetInstance() {
        static ArucoManager instance;
        return instance;
    }

    bool InitImpl();
    void DestroyImpl();

    std::vector<ImTextureID> markers;
    static constexpr int MARKER_COUNT = 13;
};
