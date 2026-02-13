#pragma once

#include "Vec3.hpp"
#include "Vec2.hpp"
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

struct view_matrix_t {
    float* operator[ ](int index) {
        return matrix[index];
    }

    float matrix[4][4];

	// World To Screen
	bool wts(const Vec3_t& pos, const Vec2_t& screen, Vec2_t& out) {
		float SightX = screen.x / 2.0f;
		float SightY = screen.y / 2.0f;

#ifdef _WIN32
		auto bounds_rect = RECT{
			0, 0,
			(LONG)screen.x,
			(LONG)screen.y
		};
		SightX = (float)bounds_rect.right / 2.0f;
		SightY = (float)bounds_rect.bottom / 2.0f;
#endif

		// A bit artificial, but its made by botikes 
		static int margin = 0;

		float view = matrix[3][0] * pos.x + matrix[3][1] * pos.y + matrix[3][2] * pos.z + matrix[3][3];

		if (view <= 0.01f)
			return false;

		out.x = SightX + (matrix[0][0] * pos.x + matrix[0][1] * pos.y + matrix[0][2] * pos.z + matrix[0][3]) / view * SightX;
		out.y = SightY - (matrix[1][0] * pos.x + matrix[1][1] * pos.y + matrix[1][2] * pos.z + matrix[1][3]) / view * SightY;

		if (
			out.x < 0.0f - (float)margin || out.x > screen.x + (float)margin ||
			out.y < 0.0f - (float)margin || out.y > screen.y + (float)margin
			)
			return false;

		return true;
	}
};