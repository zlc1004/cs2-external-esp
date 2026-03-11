#pragma once

namespace cfg {
	inline bool enabled = true;

	namespace esp {
		inline bool team = true;

		inline bool box = true;
		inline bool armor = true;
		inline bool health = true;
		inline bool skeleton = true;
		inline bool head_tracker = true;

		inline bool spotted = false;
		inline bool aruco_markers = false;

		namespace flags {
			inline bool name = true;
			inline bool ping = true;
			inline bool weapon = false;
			inline bool defusing = false;
			inline bool money = false;
			inline bool flashed = false;
			inline bool scoped = false;
		}

		namespace colors {
			inline color_t box_team{ 0.f, 1.f, 0.29f, 0.5f };	
			inline color_t box_enemy{ 1.f, 0.f, 0.f, 0.5f };

			inline color_t skeleton_team{ 0.f, 1.f, 0.f, 0.5f };
			inline color_t skeleton_enemy{ 1.f, 0.f, 0.f, 0.5f };

			inline color_t tracker_team{ 1.f, 1.f, 1.f, 0.3f };
			inline color_t tracker_enemy{ 1.f, 1.f, 1.f, 0.3f };
		}
	}

	namespace aimbot {
		inline bool enabled = false;
		inline bool rcs = false;
		inline bool vis_check = false;
		inline bool ignore_team = true;
		inline float fov = 5.0f;
		inline float smooth = 1.0f;
		inline int bone = 6; // head
		
		// Anti-recoil settings
		inline bool anti_recoil = false;       // Enable pattern-based anti-recoil
		inline bool auto_rebound = true;       // Automatically return to initial position after spray
		inline float pattern_scale = 1.0f;    // Scale factor for recoil pattern (adjust for sensitivity)
	}

	namespace settings {
		inline bool watermark = true;
		inline bool crosshair = false;
		inline bool streamproof = false;
		inline bool vsync = false;
	}

#ifdef _DEBUG
	// Not stored, just for testing
	namespace dev {
		inline bool console = true;
		inline int open_menu_key = false;
		inline int cache_refresh_rate = 5;
	}
#endif
}