#pragma once
#include "sdk.h"

class Player
{
public:
	static Player* GetPlayerAt(int i) {
		if (!globals::BASE) return nullptr;
		return (Player*)globals::BASE + (i * offsets::PLAYER_SIZE);
	}
	Vector3 getOrigin() {
		uint64_t posptr = Read<uint64_t>((uintptr_t)this + offsets::PLAYER_POS);
		return Read<Vector3>(posptr + 0x40);
	}
	uint32_t getIndex() {
		return ((uintptr_t)this - globals::BASE) / offsets::PLAYER_SIZE;
	}
	NameEntry getNameEntry() {
		auto i = getIndex();
		return Read<NameEntry>(globals::NAMES + (i * offsets::NAME_SIZE));
	}
	bool isValid() {
		uint32_t d1 = Read<uint32_t>((uintptr_t)this + offsets::PLAYER_DEAD_1);
		uint32_t d2 = Read<uint32_t>((uintptr_t)this + offsets::PLAYER_DEAD_2);
		uint32_t v = Read<uint32_t>((uintptr_t)this + offsets::PLAYER_VALID);

		return !d1 && !d2 && v == 1;
	}
};