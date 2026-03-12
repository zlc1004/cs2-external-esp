#include "Game.hpp"

#include "core/engine/Engine.hpp"
#include "core/offsets/Dumper.hpp"

bool Game::Update() {

	if (!Engine::GetProcess())
		return false;

	if (!UpdateMatrix()) {
		LOGF(FATAL, "Failed to update view matrix");
		return false;
	}

	if (!UpdateEntityList()) {
		LOGF(FATAL, "Failed to update entity list");
		return false;
	}

	return true;
}

bool Game::UpdateMatrix() {
	auto p = Engine::GetProcess();
	auto client = Engine::GetClient();

	this->view_matrix = p->read<view_matrix_t>(client.base + offsets::viewMatrix);

	return true;
}

bool Game::UpdateEntityList() {
	auto p = Engine::GetProcess();
	auto client = Engine::GetClient();

	this->entity_list = p->read<DWORD64>(client.base + offsets::entityList);
	this->list_entry = p->read<DWORD64>(this->entity_list + 0x10);

	return true;
}

bool Game::SetViewAngle(float pitch, float yaw) {
	auto p = Engine::GetProcess();
	auto client = Engine::GetClient();
	
	if (!p || client.base == 0 || offsets::viewAngles == 0)
		return false;
	
	struct Vec2 {
		float x, y;
	};
	
	Vec2 angle{ pitch, yaw };
	
	// Use protected write with VirtualProtectEx
	return p->write_protected<Vec2>(client.base + offsets::viewAngles, angle);
}