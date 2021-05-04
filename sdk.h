#pragma once
#include "driver.h"
#include "vectors.h"
#include "xor.h"

/* ==== STRUCTS ===================*/
struct RefdefView {
	Vector2 tanHalfFov;		// 0x00
	uint8_t unk1[0xC];		// 0x08
	Vector3 axis[3];      // 0x14
};

struct refdef_t {
	int x;           // 0x00
	int y;           // 0x04
	int width;       // 0x08
	int height;      // 0x0C
	RefdefView view; // 0x10
};

struct refdefKeyStruct
{
	DWORD ref0; // 0x00
	DWORD ref1; // 0x04
	DWORD ref2; // 0x08
};

struct NameEntry {
	uint32_t idx;
	char name[0x24];
	uint8_t unk1[0x64];
	uint32_t health;
};
/* =================================*/

namespace utils {
	bool is_bad_ptr(void* adr) { return adr == nullptr; }
	void log(char* str) {
		std::cout << str << std::endl;
	}
	void loghex(char* str, uint64_t val) {
		std::cout << str << std::hex << val << std::endl;
	}
}
namespace globals {
	uint64_t module_base = 0;
	uint64_t peb = 0;

	uint64_t INFO = 0;
	uint64_t BASE = 0;
	uint64_t REFDEF = 0;
	uint64_t NAMES = 0;

	const char* gn = "Rtijws\\fwkfwj3j}j";
	char* decGn() {
		char* dec = new char[18];
		for (int i = 0; i < 17; i++) {
			dec[i] = gn[i] - 5;
		}
		dec[17] = '\0';
		return dec;
	}
}
namespace offsets {
	/*
	* Short Explanation of Offsets
	* ----------------------------
	* ClientInfo is what contains a pointer to 
	* ClientBase, which is essentially the entity 
	* list and can be indexed with PLAYER_SIZE.
	* 
	* RefDef contains view related info like
	* FOV and projection matrix; also encrypted 
	* 
	* Pointers to Client Info, Client Base, and
	* RefDef are all encrypted and must first be decrypted
	* 
	* Camera contains the local player's world location
	* 
	* Name List contains both player name and health
	* and is indexed using the same index number from ClientBase
	* 
	* Sig Credits: @rmccrystal
	* Decryption Credits: @moleskn
	*/

	// 48 8B 1D ? ? ? ? C6 44 24 ? ? 0F B6 44 24 ?
	constexpr auto CLIENT_INFO = 0x1720EB88;
	// 48 8B 83 ?? ?? ?? ?? C6 44 24 ?? ?? 0F B6
	constexpr auto CLIENT_BASE = 0x9DBE8;

	// 4C 8D 1D ? ? ? ? 44 8B 15 ? ? ? ? 48 8D 1D ? ? ? ? 4C 8B C9
	constexpr auto REFDEF = 0x17211518;
	// 48 8B 05 ? ? ? ? 48 8B 7C 24 ? 48 05 ? ? ? ?
	constexpr auto CAMERA_BASE = 0x144EE700;
	constexpr auto CAMERA_POS = 0x1D8;

	// C7 83 ? ? ? ? ? ? ? ? C7 83 ? ? ? ? ? ? ? ? E8 ? ? ? ? 44 0F B6 C6 48 8B D5 48 8B CF E8 ? ? ? ?
	constexpr auto PLAYER_DEAD_1 = 0x14B8;
	// 41 83 B8 ? ? ? ? ? 0F 85 ? ? ? ? 41 B8 ? ? ? ?
	constexpr auto PLAYER_DEAD_2 = 0x9B0;
	// 49 8B D9 41 0F B6 F0 8B F9 48 8B EA
	constexpr auto PLAYER_POS = 0x4C8;
	// 48 69 D3 ?? ?? ?? ?? 48 03 96 ?? ?? ?? ??
	constexpr auto PLAYER_SIZE = 0x3A88;
	// 8B 87 ? ? ? ? 4C 8B BC 24 ? ? ? ? 4C 8B B4 24 ? ? ? ? 4C 8B AC 24 ? ? ? ? 4C 8B A4 24 ? ? ? ? 85 C0 74 16
	constexpr auto PLAYER_TEAM = 0xB14;
	// C7 87 ?? ?? ?? ?? ?? ?? ?? ?? C7 87 ?? ?? ?? ?? ?? ?? ?? ?? 41
	constexpr auto PLAYER_VALID = 0x3E4;

	// 48 8D 0D ? ? ? ? 48 8B 0C C1 48 8B 01 FF 90 ? ? ? ?
	constexpr auto NAME_LIST = 0x1721C5F8;
	constexpr auto NAME_OFFSET = 0x4C70;
	constexpr auto NAME_SIZE = 0xD0;
}
namespace decryption
{
	auto DecryptClientInfo(uint64_t enc_client) -> uint64_t
	{
		uint64_t rax = 0, rbx = 0, rcx = 0, r8 = 0;
		rbx = enc_client;

		r8 = globals::peb;
		rax = (globals::module_base + 0x777B);
		r8 *= rax;
		rax = 0x1F8F946E8C369BB;
		rbx *= rax;
		r8 ^= rbx;
		rax = r8;
		rax >>= 0x28;
		r8 ^= rax;
		rax = r8;
		rax >>= 0xE;
		r8 ^= rax;
		rax = r8;
		rax >>= 0x1C;
		r8 ^= rax;
		rax = r8;
		rax >>= 0x38;
		r8 ^= rax;
		rax = (globals::module_base + 0xFC0);
		rcx -= rax;
		rax = r8;
		//rcx &= 0xffffffffc0000000;
		rcx = 0;
		rax >>= 0x25;
		rcx <<= 0x10;
		rax ^= r8;
		rcx ^= Read<uint64_t>(globals::module_base + 0x66FA0FC);
		rcx = (~rcx);
		rbx = Read<uint64_t>(rcx + 0x9);
		rbx *= rax;
		return rbx;
	}

	auto DecryptClientBase(uint64_t enc_client_info_base) -> uint64_t
	{
		uint64_t RAX = globals::module_base, RBX = globals::module_base, RCX = globals::module_base, RDX = globals::module_base, R8 = globals::module_base, RDI = globals::module_base, R9 = globals::module_base, R10 = globals::module_base, R11 = globals::module_base, R12 = globals::module_base, R13 = globals::module_base, R14 = globals::module_base, R15 = globals::module_base, RSI = globals::module_base, RSP = globals::module_base, RBP = globals::module_base;
		RAX = enc_client_info_base;
		if (!RAX)
			return 0;
		RBX = globals::peb;
		RBX = (~RBX);
		// test rax,rax
		// je near ptr 0000000001C417DFh
		RCX = RBX;
		RCX = _rotl64(RCX, 0x25);
		// and ecx,0Fh
		RCX &= 0xF;
		switch (RCX)
		{
			case 0:
			{
				RDI = globals::module_base + 0x1D4;
				R14 = globals::module_base + 0x3FF6;
				R9 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RCX = R14;
				RCX -= RBX;
				RAX += RCX;
				RCX = RAX;
				RCX >>= 0x1A;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x34;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x6;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0xC;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x18;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x30;
				RAX ^= RCX;
				RCX = 0x890F42B7FCD615;
				RAX *= RCX;
				RCX = RAX;
				RCX >>= 0x20;
				RAX ^= RCX;
				// mov rcx,[rbp+108h]
				RCX -= RDI;
				RCX = 0; // Special case
				RCX = _rotl64(RCX, 0x10);
				RCX ^= R9;
				RCX = (~RCX);
				if (utils::is_bad_ptr((void*)(RCX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RCX = Read<uint64_t>(RCX + 0x17);
				RAX *= RCX;
				RCX = 0x71294A8B070B9839;
				RAX -= RCX;
				RCX = 0xBF1E74D2D72983AB;
				RAX *= RCX;
				return RAX;
			}
			case 1:
			{
				R11 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RDI = globals::module_base + 0x1D4;
				RDX = globals::module_base + 0x28554F16;
				RCX = RBX;
				RCX ^= RDX;
				RAX -= RCX;
				RCX = RAX;
				RCX >>= 0xE;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x1C;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x38;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0xA;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x14;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x28;
				RAX ^= RCX;
				RCX = 0xE1FD3C86DB19EBF9;
				RAX *= RCX;
				// mov r8,[rbp+108h]
				RDX = globals::module_base + 0x5D7E5CC0;
				R8 -= RDI;
				R8 = 0; // Special case
				R8 = _rotl64(R8, 0x10);
				R8 ^= R11;
				RCX = RBX;
				R8 = (~R8);
				RCX = (~RCX);
				RDX = (~RDX);
				RCX += RAX;
				RDX += RCX;
				RCX = 0x18DB6BC94B2CBA7C;
				if (utils::is_bad_ptr((void*)(R8 + 0x17))) return 0xFFFFFFFFFFFFFFFF; RAX = Read<uint64_t>(R8 + 0x17);
				RAX *= RDX;
				RAX ^= RCX;
				RCX = 0x68C819A3C9078EC0;
				RAX ^= RCX;
				return RAX;
			}
			case 2:
			{
				R10 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RDI = globals::module_base + 0x1D4;
				R15 = globals::module_base + 0xD51F;
				R11 = globals::module_base;
				RCX = RAX;
				RCX >>= 0x11;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x22;
				RAX ^= RCX;
				// mov rdx,[rbp+108h]
				RDX -= RDI;
				RDX = 0; // Special case
				RCX = RAX + R11;
				RDX = _rotl64(RDX, 0x10);
				RDX ^= R10;
				RDX = (~RDX);
				if (utils::is_bad_ptr((void*)(RDX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RAX = Read<uint64_t>(RDX + 0x17);
				RAX *= RCX;
				RCX = 0x4DE64FAFA04AFCC3;
				RAX *= RCX;
				RCX = R15;
				RCX = (~RCX);
				RCX += RBX;
				RAX ^= RCX;
				RAX -= RBX;
				RCX = 0x9EF4528D17D101CD;
				RAX ^= RCX;
				return RAX;
			}
			case 3:
			{
				RDI = globals::module_base + 0x1D4;
				R11 = globals::module_base;
				R9 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RCX = 0x51DF0317C63BA0B0;
				RAX -= RCX;
				RCX = RAX;
				RCX >>= 0x4;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x8;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x10;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x20;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0xC;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x18;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x30;
				RAX ^= RCX;
				RCX = 0x7B440B4A6B0A8EF5;
				RAX *= RCX;
				// mov rcx,[rbp+108h]
				RCX -= RDI;
				RCX = 0; // Special case
				RCX = _rotl64(RCX, 0x10);
				RCX ^= R9;
				RCX = (~RCX);
				if (utils::is_bad_ptr((void*)(RCX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RCX = Read<uint64_t>(RCX + 0x17);
				RAX *= RCX;
				RAX -= RBX;
				RAX ^= R11;
				RCX = 0x3762E5D919DF6CF0;
				RAX -= RCX;
				return RAX;
			}
			case 4:
			{
				RDI = globals::module_base + 0x1D4;
				R11 = globals::module_base;
				R15 = globals::module_base + 0x455B9C25;
				R9 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RCX = RBX;
				RBP = globals::module_base + 0x949;
				RCX ^= RBP;
				RAX += RCX;
				// mov rcx,[rbp+108h]
				RCX -= RDI;
				RCX = 0; // Special case
				RCX = _rotl64(RCX, 0x10);
				RCX ^= R9;
				RCX = (~RCX);
				if (utils::is_bad_ptr((void*)(RCX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RCX = Read<uint64_t>(RCX + 0x17);
				RSP = 0xDC393A1CB26C59A9;
				RCX *= RSP;
				RAX *= RCX;
				RAX -= RBX;
				RCX = RBX;
				RCX ^= R15;
				RAX -= RCX;
				RCX = RAX;
				RCX >>= 0x11;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x22;
				RAX ^= RCX;
				RAX -= R11;
				RAX += R11;
				return RAX;
			}
			case 5:
			{
				RDI = globals::module_base + 0x1D4;
				R11 = globals::module_base + 0x739C6080;
				R9 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RCX = 0xE56F47E25078E80C;
				RAX ^= RCX;
				RCX = 0x36BCDD71AC89676F;
				RAX *= RCX;
				RCX = RAX;
				RCX >>= 0xD;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x1A;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x34;
				RAX ^= RCX;
				RCX = R11;
				RCX = (~RCX);
				RCX ^= RBX;
				RAX ^= RCX;
				RAX += RBX;
				RCX = 0xC824B36FD4F56BF3;
				RAX *= RCX;
				// mov rcx,[rbp+108h]
				RCX -= RDI;
				RCX = 0; // Special case
				RCX = _rotl64(RCX, 0x10);
				RCX ^= R9;
				RCX = (~RCX);
				if (utils::is_bad_ptr((void*)(RCX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RAX *= Read<uint64_t>(RCX + 0x17);
				RCX = RAX;
				RCX >>= 0xA;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x14;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x28;
				RAX ^= RCX;
				return RAX;
			}
			case 6:
			{
				R10 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RDI = globals::module_base + 0x1D4;
				R11 = globals::module_base;
				RCX = 0x2EBB8A785E1A856A;
				RAX += RCX;
				RAX ^= R11;
				RCX = 0xC5EE09076CC9FE1C;
				RAX ^= RCX;
				RAX -= RBX;
				RCX = RAX;
				RCX >>= 0x1B;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x36;
				RAX ^= RCX;
				RCX = 0x1AEFC88777814F31;
				RAX *= RCX;
				// mov rcx,[rbp+108h]
				RCX -= RDI;
				RCX = 0; // Special case
				RCX = _rotl64(RCX, 0x10);
				RCX ^= R10;
				RCX = (~RCX);
				if (utils::is_bad_ptr((void*)(RCX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RAX *= Read<uint64_t>(RCX + 0x17);
				RAX -= RBX;
				return RAX;
			}
			case 7:
			{
				R10 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RDI = globals::module_base + 0x1D4;
				R15 = globals::module_base + 0x39BD428F;
				RCX = 0x4A8E836720C9B049;
				RAX += RCX;
				RAX ^= RBX;
				RAX ^= R15;
				RCX = 0x47ADAA0C40AA79CE;
				RAX += RCX;
				RAX += RBX;
				RCX = RAX;
				RCX >>= 0x13;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x26;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x18;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x30;
				RAX ^= RCX;
				RCX = 0x7404B5F5DE776B17;
				RAX *= RCX;
				// mov rcx,[rbp+108h]
				RCX -= RDI;
				RCX = 0; // Special case
				RCX = _rotl64(RCX, 0x10);
				RCX ^= R10;
				RCX = (~RCX);
				if (utils::is_bad_ptr((void*)(RCX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RAX *= Read<uint64_t>(RCX + 0x17);
				return RAX;
			}
			case 8:
			{
				RDI = globals::module_base + 0x1D4;
				R11 = globals::module_base;
				R10 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RDX = RBX;
				RCX = globals::module_base + 0x1818822A;
				RDX *= RCX;
				RCX = R11;
				RCX -= RDX;
				RAX += RCX;
				RCX = 0xF3DCF6EBF3A3997;
				RAX += RCX;
				RCX = 0x4C42D15E4A2708E5;
				RAX *= RCX;
				// mov rcx,[rbp+108h]
				RCX -= RDI;
				RCX = 0; // Special case
				RCX = _rotl64(RCX, 0x10);
				RCX ^= R10;
				RCX = (~RCX);
				if (utils::is_bad_ptr((void*)(RCX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RAX *= Read<uint64_t>(RCX + 0x17);
				RCX = 0x83DBAD3327E0A500;
				RAX ^= RCX;
				RAX -= RBX;
				RCX = RAX;
				RCX >>= 0x12;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x24;
				RAX ^= RCX;
				return RAX;
			}
			case 9:
			{
				RDI = globals::module_base + 0x1D4;
				R11 = globals::module_base;
				R10 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RCX = R11 + 0xF50;
				RCX += RBX;
				RAX += RCX;
				RAX ^= RBX;
				// mov rdx,[rbp+108h]
				RDX -= RDI;
				RDX = 0; // Special case
				RCX = 0x9475C968613A0C67;
				RDX = _rotl64(RDX, 0x10);
				RCX += RAX;
				RDX ^= R10;
				RDX = (~RDX);
				if (utils::is_bad_ptr((void*)(RDX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RAX = Read<uint64_t>(RDX + 0x17);
				RAX *= RCX;
				RCX = globals::module_base + 0x6E91D3AB;
				RCX = (~RCX);
				RCX *= RBX;
				RAX ^= RCX;
				RCX = 0x1A81BB2AD0B4F3AC;
				RAX -= RCX;
				RCX = 0xBBB2C4BB8CE6593;
				RAX *= RCX;
				RCX = RAX;
				RCX >>= 0x19;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x32;
				RAX ^= RCX;
				return RAX;
			}
			case 10:
			{
				R9 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RDI = globals::module_base + 0x1D4;
				R11 = globals::module_base;
				R15 = globals::module_base + 0x6EA15250;
				RAX += RBX;
				RBP = 0x9BD059290DCB43D3;
				RAX *= RBP;
				RCX = RBX;
				RCX = (~RCX);
				RCX ^= R15;
				RAX -= RCX;
				RAX ^= RBX;
				RAX -= R11;
				// mov rcx,[rbp+108h]
				RCX -= RDI;
				RCX = 0; // Special case
				RCX = _rotl64(RCX, 0x10);
				RCX ^= R9;
				RCX = (~RCX);
				if (utils::is_bad_ptr((void*)(RCX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RAX *= Read<uint64_t>(RCX + 0x17);
				RCX = RAX;
				RCX >>= 0x8;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x10;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x20;
				RAX ^= RCX;
				RAX -= RBX;
				return RAX;
			}
			case 11:
			{
				RDI = globals::module_base + 0x1D4;
				R11 = globals::module_base;
				R10 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RCX = RAX;
				RCX >>= 0x14;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x28;
				RAX ^= RCX;
				RCX = 0x60FC131021EA9670;
				RAX ^= RCX;
				RCX = R11 + 0x8054;
				RCX += RBX;
				RAX += RCX;
				RAX ^= R11;
				// mov rdx,[rbp+108h]
				RDX -= RDI;
				RDX = 0; // Special case
				RCX = RAX;
				RDX = _rotl64(RDX, 0x10);
				RCX ^= R11;
				RDX ^= R10;
				RDX = (~RDX);
				if (utils::is_bad_ptr((void*)(RDX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RAX = Read<uint64_t>(RDX + 0x17);
				RAX *= RCX;
				RCX = 0x989F1826B9D2513F;
				RAX *= RCX;
				return RAX;
			}
			case 12:
			{
				R10 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RDI = globals::module_base + 0x1D4;
				R11 = globals::module_base;
				RCX = globals::module_base + 0x56E1284B;
				RCX = (~RCX);
				RCX -= RBX;
				RAX ^= RCX;
				RCX = globals::module_base + 0xD4EC;
				RCX = (~RCX);
				RCX -= RBX;
				RAX += RCX;
				RCX = 0xBBA27374E8361FEB;
				RAX *= RCX;
				RCX = RAX;
				RCX >>= 0x1C;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x38;
				RAX ^= RCX;
				RCX = R11 + 0x42C2;
				RCX += RBX;
				RAX ^= RCX;
				// mov rcx,[rbp+108h]
				RCX -= RDI;
				RCX = 0; // Special case
				RCX = _rotl64(RCX, 0x10);
				RCX ^= R10;
				RCX = (~RCX);
				if (utils::is_bad_ptr((void*)(RCX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RAX *= Read<uint64_t>(RCX + 0x17);
				RCX = globals::module_base + 0x4FE2BA2B;
				RDX = RBX;
				RAX += RCX;
				RDX = (~RDX);
				RAX += RDX;
				RCX = 0x1EE121203251119E;
				RAX += RCX;
				return RAX;
			}
			case 13:
			{
				RDI = globals::module_base + 0x1D4;
				R11 = globals::module_base;
				R10 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				// mov rcx,[rbp+108h]
				RCX -= RDI;
				RCX = 0; // Special case
				RCX = _rotl64(RCX, 0x10);
				RCX ^= R10;
				RCX = (~RCX);
				if (utils::is_bad_ptr((void*)(RCX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RAX *= Read<uint64_t>(RCX + 0x17);
				RAX += RBX;
				RAX -= R11;
				RCX = 0xF2C3607F255C85D7;
				RAX *= RCX;
				RCX = 0xA0E2B3FEB404B52;
				RAX ^= RCX;
				RCX = RBX;
				RBP = globals::module_base + 0x17773B74;
				RCX *= RBP;
				RAX += RCX;
				RCX = RAX;
				RCX >>= 0x4;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x8;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x10;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x20;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x1E;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x3C;
				RAX ^= RCX;
				return RAX;
			}
			case 14:
			{
				RDI = globals::module_base + 0x1D4;
				R11 = globals::module_base;
				R9 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RAX -= RBX;
				RAX -= R11;
				RAX -= 0x8DEB;
				RAX ^= RBX;
				// mov rcx,[rbp+108h]
				RCX -= RDI;
				RCX = 0; // Special case
				RCX = _rotl64(RCX, 0x10);
				RCX ^= R9;
				RCX = (~RCX);
				if (utils::is_bad_ptr((void*)(RCX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RCX = Read<uint64_t>(RCX + 0x17);
				RSP = 0x87F2757EC1B54FAB;
				RCX *= RSP;
				RAX *= RCX;
				RCX = RAX;
				RCX >>= 0xD;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x1A;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x34;
				RAX ^= RCX;
				RCX = globals::module_base + 0x7E28C7A5;
				RCX -= RBX;
				RAX += RCX;
				RAX -= R11;
				return RAX;
			}
			case 15:
			{
				R10 = Read<uint64_t>(globals::module_base + 0x66FA12A);
				RDI = globals::module_base + 0x1D4;
				RCX = 0x73482614CEAA9160;
				RAX ^= RCX;
				RAX += RBX;
				RCX = RAX;
				RCX >>= 0xB;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x16;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x2C;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x12;
				RAX ^= RCX;
				RCX = RAX;
				RCX >>= 0x24;
				RAX ^= RCX;
				// mov rcx,[rbp+108h]
				RCX -= RDI;
				RCX = 0; // Special case
				RCX = _rotl64(RCX, 0x10);
				RCX ^= R10;
				RCX = (~RCX);
				if (utils::is_bad_ptr((void*)(RCX + 0x17))) return 0xFFFFFFFFFFFFFFFF; RAX *= Read<uint64_t>(RCX + 0x17);
				RCX = 0xFD678AA3934E2FC7;
				RAX *= RCX;
				RCX = globals::module_base + 0x80E2;
				RAX += RCX;
				return RAX;
			}
			default:
				return 0;
		}
	}

	auto DecryptRefDef(refdefKeyStruct crypt)
	{
		uint64_t baseAddr = globals::module_base;

		DWORD lower = crypt.ref0 ^ (crypt.ref2 ^ (uint64_t)(baseAddr + offsets::REFDEF)) * ((crypt.ref2 ^ (uint64_t)(baseAddr + offsets::REFDEF)) + 2);
		DWORD upper = crypt.ref1 ^ (crypt.ref2 ^ (uint64_t)(baseAddr + offsets::REFDEF + 0x4)) * ((crypt.ref2 ^ (uint64_t)(baseAddr + offsets::REFDEF + 0x4)) + 2);

		return (uint64_t)upper << 32 | lower; // Merge Both DWORD into QWORD
	}
}

uint64_t GetRefDef()
{
	auto encrefdef = Read<refdefKeyStruct>(globals::module_base + offsets::REFDEF);
	return decryption::DecryptRefDef(encrefdef);
}
uint64_t GetClientInfo() {
	auto encinfo = Read<uint64_t>(globals::module_base + offsets::CLIENT_INFO);
	return decryption::DecryptClientInfo(encinfo);
}
uint64_t GetClientBase(uint64_t client_info) {
	auto encbase = Read<uint64_t>(client_info + offsets::CLIENT_BASE);
	return decryption::DecryptClientBase(encbase);
}
uint64_t GetNameList() {
	auto ptr = Read<uint64_t>(globals::module_base + offsets::NAME_LIST);
	return ptr + offsets::NAME_OFFSET;
}









