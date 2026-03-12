#include "Menu.hpp"

#include "core/engine/cache/Cache.hpp"
#include "gui/renderer/Renderer.hpp" // Circular dependency
#include "gui/renderer/window/Window.hpp" // Circular dependency


bool Menu::Init() {
    return GetInstance().InitImpl();
}

void Menu::Render() {
    return GetInstance().RenderImpl();
}

void Menu::RenderStartupHelp() {
	return GetInstance().RenderStartupHelpImpl();
}

ImVec2 Menu::GetPos() {
	return GetInstance().pos;
}

ImVec2 Menu::GetSize() {
	return GetInstance().size;
}

bool Menu::InitImpl() {
	SetupStyles();

    LOGF(INFO, "Successfully initialized menu...");
    return true;
}

void Menu::RenderImpl() {
	if (!isSetup)
		return;

	static auto io = ImGui::GetIO();
	static auto screen = io.DisplaySize;
	static auto color_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None;

#ifdef _DEBUG
	static auto title = "github.com/IMXNOOBX/cs2-external-esp (recode) [DEV]";
#else
	static auto title = "cs2-external-esp | recode (PTB)";
#endif

	ImGui::SetNextWindowSize(ImVec2(600, 350), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(screen.x/2 - 300, screen.y/2 - 150), ImGuiCond_FirstUseEver);

	ImGui::GetWindowPos();
	if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
		this->pos = ImGui::GetWindowPos();
		this->size = ImGui::GetWindowSize();

		ImGui::Checkbox("Enable", &cfg::enabled);
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(340, 0));
		ImGui::SameLine();
		ImGui::TextLinkOpenURL("Source/Support", "https://github.com/IMXNOOBX/cs2-external-esp");
		ImGui::SameLine();
		ImGui::TextLinkOpenURL("Discord", "https://discord.gg/pRew8ZDkyp");

		static int y_space_left;
		auto space = ImGui::GetContentRegionAvail();

		ImGui::BeginDisabled(!cfg::enabled);
		if (ImGui::BeginChild("##top", ImVec2(0, space.y / 2 + y_space_left)))
		{
			if (ImGui::BeginChild("##leftside", ImVec2(space.x / 2 - 10, 0)))
			{
				ImGui::Text("Visuals");
				ImGui::Separator();

				ImGui::BeginGroup();
				{
					ImGui::Checkbox("Box", &cfg::esp::box);
					ImGui::BeginDisabled(!cfg::esp::box);
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("Team box color", cfg::esp::colors::box_team.data(), color_flags);
						ImGui::SetItemTooltip("Team box color");

						ImGui::SameLine();
						ImGui::ColorEdit4("Enemy box color", cfg::esp::colors::box_enemy.data(), color_flags);
						ImGui::SetItemTooltip("Enemy box color");
					}
					ImGui::EndDisabled();
					
					ImGui::Checkbox("Skeleton", &cfg::esp::skeleton);
					ImGui::BeginDisabled(!cfg::esp::skeleton);
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("Team skeleton color", cfg::esp::colors::skeleton_team.data(), color_flags);
						ImGui::SetItemTooltip("Team skeleton color");

						ImGui::SameLine();
						ImGui::ColorEdit4("Enemy skeleton color", cfg::esp::colors::skeleton_enemy.data(), color_flags);
						ImGui::SetItemTooltip("Enemy skeleton color");
					}
					ImGui::EndDisabled();

					ImGui::Checkbox("Head Tracker", &cfg::esp::head_tracker);
					ImGui::BeginDisabled(!cfg::esp::head_tracker);
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("Team head tracker color", cfg::esp::colors::tracker_team.data(), color_flags);
						ImGui::SetItemTooltip("Team head tracker color");

						ImGui::SameLine();
						ImGui::ColorEdit4("Enemy head tracker color", cfg::esp::colors::tracker_enemy.data(), color_flags);
						ImGui::SetItemTooltip("Enemy head tracker color");
					}
					ImGui::EndDisabled();

					ImGui::Checkbox("Show Team", &cfg::esp::team);
				}
				ImGui::EndGroup();

				ImGui::SameLine();

				ImGui::BeginGroup();
				{
					ImGui::Checkbox("Health", &cfg::esp::health);
					ImGui::Checkbox("Armor", &cfg::esp::armor);
					ImGui::Checkbox("Spotted", &cfg::esp::spotted);
					ImGui::SetItemTooltip("Esp will only be visible if the player has been spotted by you");
					
					ImGui::Checkbox("ArUco Markers", &cfg::esp::aruco_markers);
					ImGui::SetItemTooltip("Display ArUco markers on player heads (requires marker images in aruco/ folder)");
				}
				ImGui::EndGroup();

				ImGui::Text("Aimbot & Recoil Control");
				ImGui::Separator();
				
				// Show current weapon info for debugging
				auto cache = Cache::CopySnapshot();
				for (auto& player : cache.players) {
					if (player.localplayer && player.alive) {
						ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Current Weapon: %s", player.clean_weapon.c_str());
						ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Shots Fired: %d", player.shots_fired);
						break;
					}
				}
				
				// Anti-Recoil section (works independently)
				ImGui::Text("Pattern-Based Anti-Recoil");
				ImGui::Separator();
				ImGui::Checkbox("Anti-Recoil##standalone", &cfg::aimbot::anti_recoil);
				ImGui::SetItemTooltip("Pattern-based recoil compensation - triggers on LEFT MOUSE BUTTON\nWorks independently, doesn't require aimbot enabled!");
				
				// Show anti-recoil options if enabled
				if (cfg::aimbot::anti_recoil) {
					ImGui::Indent();
					ImGui::Checkbox("Auto Rebound##antirecoil", &cfg::aimbot::auto_rebound);
					ImGui::SetItemTooltip("Automatically return to initial position after spray ends");
					
					ImGui::SliderFloat("Pattern Scale##antirecoil", &cfg::aimbot::pattern_scale, 0.5f, 2.0f, "%.2f");
					ImGui::SetItemTooltip("Adjust pattern intensity for your sensitivity\nDefault 1.0x = sens 2.40-2.60");
					
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Supported: AK47, M4A4, M4A1-S, FAMAS,");
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "GALIL, AUG, P90");
					
					// Show active status
					if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
						ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "STATUS: ACTIVE (Shooting)");
					} else {
						ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "STATUS: Standby (Hold LMB to activate)");
					}
					
					ImGui::Unindent();
				}
				
				ImGui::Spacing();
				ImGui::Spacing();
				
				// Aimbot section
				ImGui::Text("Aimbot");
				ImGui::Separator();
				ImGui::Checkbox("Aimbot Enabled", &cfg::aimbot::enabled);
				ImGui::BeginDisabled(!cfg::aimbot::enabled);
				{
					ImGui::Checkbox("RCS", &cfg::aimbot::rcs);
					ImGui::SetItemTooltip("Standard Recoil Control System (aim punch based)");
					
					if (cfg::aimbot::rcs) {
						ImGui::SliderFloat("RCS Scale", &cfg::aimbot::rcs_scale, 0.0f, 2.0f, "%.2f");
						ImGui::SetItemTooltip("Recoil compensation strength (1.0 = full compensation)");
					}
					
					ImGui::Checkbox("Visibility Check", &cfg::aimbot::vis_check);
					ImGui::Checkbox("Ignore Team", &cfg::aimbot::ignore_team);
					ImGui::SliderFloat("FOV", &cfg::aimbot::fov, 0.1f, 100.0f, "%.1f");
					ImGui::SetItemTooltip("Field of view for aimbot activation (degrees)");
					ImGui::Checkbox("Draw FOV Circle", &cfg::aimbot::draw_fov);
					ImGui::SetItemTooltip("Show FOV circle on screen");
					
					const char* bones[] = { "Head", "Neck", "Spine", "Pelvis" };
					static int selected_bone = 0;
					if (ImGui::Combo("Target Bone", &selected_bone, bones, IM_ARRAYSIZE(bones))) {
						switch (selected_bone) {
						case 0: cfg::aimbot::bone = 6; break; // head
						case 1: cfg::aimbot::bone = 5; break; // neck
						case 2: cfg::aimbot::bone = 4; break; // spine
						case 3: cfg::aimbot::bone = 0; break; // pelvis
						}
					}
				}
				ImGui::EndDisabled();
			}
			
			// TriggerBot Section
			if (ImGui::CollapsingHeader("TriggerBot", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Checkbox("TriggerBot Enabled", &cfg::triggerbot::enabled);
				ImGui::BeginDisabled(!cfg::triggerbot::enabled);
				{
					ImGui::Checkbox("Ignore Team##triggerbot", &cfg::triggerbot::ignore_team);
					ImGui::SliderInt("Delay##triggerbot", &cfg::triggerbot::delay, 10, 300, "%d ms");
					ImGui::TextWrapped("Hotkey: ALT (hold to activate)");
				}
				ImGui::EndDisabled();
				ImGui::Spacing();
			}
			
			// Bhop Section
			if (ImGui::CollapsingHeader("Bhop", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Checkbox("Bhop Enabled", &cfg::bhop::enabled);
				ImGui::TextWrapped("Automatic bunnyhopping - hold SPACE to jump repeatedly.");
				ImGui::Spacing();
			}
			
			// Anti-Flash Section  
			if (ImGui::CollapsingHeader("Anti-Flash", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Checkbox("Anti-Flash Enabled", &cfg::antiflash::enabled);
				ImGui::TextWrapped("Automatically removes flashbang effects.");
				ImGui::Spacing();
			}
			ImGui::EndChild();

			ImGui::SameLine();

			if (ImGui::BeginChild("##rightside")) 
			{
				ImGui::Text("Flags");
				ImGui::Separator();

				ImGui::BeginGroup();
				{
					ImGui::Checkbox("Name", &cfg::esp::flags::name);
					ImGui::Checkbox("Weapon", &cfg::esp::flags::weapon);
					ImGui::Checkbox("Defusing", &cfg::esp::flags::defusing);
					ImGui::Checkbox("Scoped", &cfg::esp::flags::scoped);
				}
				ImGui::EndGroup();
				
				ImGui::SameLine();

				ImGui::BeginGroup();
				{
					ImGui::Checkbox("Money", &cfg::esp::flags::money);
					ImGui::Checkbox("Flashed", &cfg::esp::flags::flashed);
					ImGui::Checkbox("Ping", &cfg::esp::flags::ping);
				}
				ImGui::EndGroup();
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();
		ImGui::EndDisabled();

		if (ImGui::BeginChild("##bottom"))
		{
			ImGui::Text("Settings");
			ImGui::Separator();

			if (ImGui::Checkbox("Streamproof", &cfg::settings::streamproof)) {
				Window::SetAffinity(
					Window::hwnd, 
					cfg::settings::streamproof ? WindowAffinity::Invisible : WindowAffinity::Disabled
				);
			}

			ImGui::Checkbox("Watermark", &cfg::settings::watermark);
			ImGui::Checkbox("Crosshair", &cfg::settings::crosshair);

			if (ImGui::Checkbox("VSync", &cfg::settings::vsync))
				Window::vsync = cfg::settings::vsync;
			ImGui::SetItemTooltip("VSync, matches render speed with screen refresh rate, improving performance");

#ifdef _DEBUG
			ImGui::Text("Dev");
			ImGui::Separator();

			if (ImGui::Checkbox("Console", &cfg::dev::console)) 
				if (!cfg::dev::console) LogHelper::Free();
			
			static int key_out;
			if (ImGui::Button("Open Menu Key")) {
				for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; i++) {
					if (ImGui::IsKeyPressed((ImGuiKey)i)) {
						key_out = i;
						LOGF(VERBOSE, "Changed the open menu key to {}", key_out);
						break;
					}
				}
			}

			ImGui::SliderInt("Cache Refresh Rate", &cfg::dev::cache_refresh_rate, 0, 100, "%dms");
#endif
		}
		ImGui::EndChild();

		y_space_left = ImGui::GetContentRegionAvail().y;
	}

	ImGui::End();
}

void Menu::SetupStyles() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

	style.Colors[ImGuiCol_WindowBg] = ImColor(10, 10, 10);
	style.Colors[ImGuiCol_ChildBg] = ImColor(10, 10, 10);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImColor(50, 50, 50);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	style.Colors[ImGuiCol_FrameBg] = ImColor(75, 75, 75);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);

	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);

	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);

	style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.29f, 0.30f, 0.31f, 0.95f);

	style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	style.FrameBorderSize = 1.0f;

	// Window & Frame
	style.WindowRounding = 12.f;
	style.ChildRounding = 10.f;

	style.FrameRounding = 5.f;
	style.PopupRounding = 5.f;

	style.GrabRounding = 3.f;

    auto& io = ImGui::GetIO();

    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f);
}

void Menu::RenderStartupHelpImpl() {
	static bool has_opened_menu = false;

	if (has_opened_menu)
		return;

	auto& io = ImGui::GetIO();
	auto screen = io.DisplaySize;
	auto d = ImGui::GetBackgroundDrawList();

	if (Renderer::IsOpen())
		has_opened_menu = true;

	auto help = "To OPEN the menu, Use Insert or Right Shift keys"
		"\n\t\t\t\tTo CLOSE, press the End key";
	auto size = ImGui::CalcTextSize(help);

	d->AddText(
		ImVec2(screen.x / 2 - size.x / 2, 80),
		IM_COL32(255, 255, 255, 255),
		help
	);
}