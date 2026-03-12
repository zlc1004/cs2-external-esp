#pragma once

#include "../types/Matrix.hpp"

class Game {
public:
    Game() {};

    bool Update();
    bool UpdateMatrix();
    bool UpdateEntityList();
    
    // NEW: SetViewAngle for recoil control (replaces mouse_event)
    bool SetViewAngle(float pitch, float yaw);
    
    // Read current view angles from dwViewAngles
    bool GetViewAngles(float& pitch, float& yaw);

public:
    view_matrix_t view_matrix;

    uintptr_t entity_list;
    uintptr_t list_entry;
private:
    uintptr_t address;
};