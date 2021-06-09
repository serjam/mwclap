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
	constexpr auto CLIENT_BASE = 0x9DC28;
	// 4C 8D 1D ? ? ? ? 44 8B 15 ? ? ? ? 48 8D 1D ? ? ? ? 4C 8B C9
	constexpr auto REFDEF = 0x812F2068;
	// 48 8B 05 ? ? ? ? 48 8B 7C 24 ? 48 05 ? ? ? ?
	constexpr auto CAMERA_BASE = 0x1B17E6020F0;
	constexpr auto CAMERA_POS = 0x1D8;
	
	// C7 83 ? ? ? ? ? ? ? ? C7 83 ? ? ? ? ? ? ? ? E8 ? ? ? ? 44 0F B6 C6 48 8B D5 48 8B CF E8 ? ? ? ?
	constexpr auto PLAYER_DEAD_1 = 0x3220;
	// 41 83 B8 ? ? ? ? ? 0F 85 ? ? ? ? 41 B8 ? ? ? ?
	constexpr auto PLAYER_DEAD_2 = 0x3D8;
	// 49 8B D9 41 0F B6 F0 8B F9 48 8B EA
	constexpr auto PLAYER_POS = 0x980; // pos_ptr
	// 48 69 D3 ?? ?? ?? ?? 48 03 96 ?? ?? ?? ??
	constexpr auto PLAYER_SIZE = 0x3AB8;
	// 8B 87 ? ? ? ? 4C 8B BC 24 ? ? ? ? 4C 8B B4 24 ? ? ? ? 4C 8B AC 24 ? ? ? ? 4C 8B A4 24 ? ? ? ? 85 C0 74 16
	constexpr auto PLAYER_TEAM = 0x2EA8;
	// C7 87 ?? ?? ?? ?? ?? ?? ?? ?? C7 87 ?? ?? ?? ?? ?? ?? ?? ?? 41
	constexpr auto PLAYER_VALID = 0x3E4;

	// 48 8D 0D ? ? ? ? 48 8B 0C C1 48 8B 01 FF 90 ? ? ? ?
	constexpr auto NAME_LIST = 0x1B17E03C558; // name array
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
		rbx += r8;
		rax = rbx;
		rax >>= 0x1F;
		rbx ^= rax;
		rax = (globals::module_base + 0xB67);
		rcx -= rax;
		rax = rbx;
		rax >>= 0x3E;
		rcx = 0;
		rax ^= r8;
		rcx <<= 0x10;
		rcx ^= Read<uint64_t>(globals::module_base + 0x6720104);
		rax ^= rbx;
		rcx = (~rcx);
		rbx = Read<uint64_t>(rcx + 0xb);
		rbx *= rax;
		rax = 0x6075DC559853C701;
		rbx *= rax;
		rax = (globals::module_base + 0x375);
		rax = (~rax);
		rax *= r8;
		rbx += rax;
		return rbx;

	}

	auto DecryptClientBase(uint64_t enc_client_info_base) -> uint64_t
	{
		uint64_t rax = globals::module_base, rbx = globals::module_base, rcx = globals::module_base, rdx = globals::module_base, r8 = globals::module_base, rdi = globals::module_base, r9 = globals::module_base, r10 = globals::module_base, r11 = globals::module_base, r12 = globals::module_base, r13 = globals::module_base, r14 = globals::module_base, r15 = globals::module_base, rsi = globals::module_base, rsp = globals::module_base, rbp = globals::module_base;
		rax = enc_client_info_base;
		if (!rax)
			return 0;
		rbx = globals::peb;
		rbx = (~rbx);
		// test rax,rax
		// je near ptr 0000000001C417DFh
		rcx = rbx;
		rcx = _rotl64(rcx, 0x25);
		// and ecx,0Fh
		rcx &= 0xF;
		switch (rcx)
		{
		case 0:
		{
			r10 = Read<uint64_t>(globals::module_base + 0x6720149);
			rsi = (globals::module_base + 0xC61);
			rbx = globals::module_base;
			rdx = Read<uint64_t>(rbp + 0xe8);
			rdx -= rsi;
			rdx = 0;
			rdx <<= 0x10;
			rcx = rax;
			rdx ^= r10;
			rax = 0xCA651297FA39DCF9;
			rcx ^= rax;
			_byteswap_uint64(rdx);
			rax = Read<uint64_t>(rdx + 0x9);
			rax *= rcx;
			rcx = 0xF2639EBCDA9E18A3;
			rax ^= rcx;
			rcx = 0x69DD05AC1525C7F9;
			rax *= rcx;
			rax ^= rdi;
			rax -= rdi;
			rax -= rbx;
			rax += 0;
			rax += rdi;
			rcx = rax;
			rcx >>= 0x25;
			rax ^= rcx;
			return rax;
			break;
		}

		case 1:
		{
			r10 = Read<uint64_t>(globals::module_base + 0x6720149);
			rsi = (globals::module_base + 0xC61);
			r15 = (globals::module_base + 0xA26B);
			rax -= rdi;
			rcx = 0xE706946ECCA84A0C;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0xD;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x1A;
			rax ^= rcx;
			rdx = Read<uint64_t>(rbp + 0xe8);
			rdx -= rsi;
			rdx = 0;
			rcx = rax;
			rdx <<= 0x10;
			rcx >>= 0x34;
			rdx ^= r10;
			rcx ^= rax;
			_byteswap_uint64(rdx);
			rax = Read<uint64_t>(rdx + 0x9);
			rax *= rcx;
			rdx = (globals::module_base + 0x7059);
			rcx = rax;
			rcx >>= 0x10;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x20;
			rax ^= rcx;
			rcx = r15;
			rcx = (~rcx);
			rcx ^= rdi;
			rax += rcx;
			rcx = 0x5A61DE826FC972BD;
			rax *= rcx;
			rcx = rdi;
			rcx ^= rdx;
			rax += rcx;
			return rax;
			break;
		}

		case 2:
		{
			rsi = (globals::module_base + 0xC61);
			rbx = globals::module_base;
			r10 = Read<uint64_t>(globals::module_base + 0x6720149);
			rax -= rdi;
			rcx = rax;
			rcx >>= 0x24;
			rax ^= rcx;
			rcx = 0x9862196F90D39563;
			rax *= rcx;
			rax -= rbx;
			rax += 0;
			rax += rdi;
			rcx = Read<uint64_t>(rbp + 0xe8);
			rcx -= rsi;
			rcx = 0;
			rcx <<= 0x10;
			rcx ^= r10;
			_byteswap_uint64(rcx);
			rax *= Read<uint64_t>(rcx + 0x9);
			rcx = 0x71F65F5BA3DA4AE5;
			rax ^= rcx;
			rax += rbx;
			rcx = 0x6C124DD872E58B5A;
			rax -= rcx;
			return rax;
			break;
		}

		case 3:
		{
			r10 = Read<uint64_t>(globals::module_base + 0x6720149);
			rsi = (globals::module_base + 0xC61);
			rbx = globals::module_base;
			rax ^= rbx;
			rax += rdi;
			rcx = rax;
			rcx >>= 0x24;
			rax ^= rcx;
			rax -= rbx;
			rcx = 0x77B6B3262338696F;
			rax *= rcx;
			rcx = 0x60C424B88DAD8E95;
			rax *= rcx;
			rcx = 0xF55D459D84BC7488;
			rax ^= rcx;
			rcx = Read<uint64_t>(rbp + 0xe8);
			rcx -= rsi;
			rcx = 0;
			rcx <<= 0x10;
			rcx ^= r10;
			_byteswap_uint64(rcx);
			rax *= Read<uint64_t>(rcx + 0x9);
			return rax;
			break;
		}

		case 4:
		{
			rsi = (globals::module_base + 0xC61);
			rbx = globals::module_base;
			r9 = Read<uint64_t>(globals::module_base + 0x6720149);
			rcx = Read<uint64_t>(rbp + 0xe8);
			rcx -= rsi;
			rcx = 0;
			rcx <<= 0x10;
			rcx ^= r9;
			_byteswap_uint64(rcx);
			rcx = Read<uint64_t>(rcx + 0x9);
			rax *= rcx;
			rax ^= rdi;
			rax -= rdi;
			rax ^= rdi;
			rax ^= rbx;
			rcx = rax;
			rcx >>= 0x24;
			rax ^= rcx;
			rcx = 0x8F36D199F791A0BB;
			rax *= rcx;
			rax ^= rdi;
			return rax;
			break;
		}

		case 5:
		{
			rsi = (globals::module_base + 0xC61);
			r10 = Read<uint64_t>(globals::module_base + 0x6720149);
			rcx = rax;
			rcx >>= 0x1B;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x36;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x27;
			rax ^= rcx;
			rcx = 0x59394AE48A41D6F2;
			rax ^= rcx;
			rcx = 0x6F91F582EB6ED077;
			rax *= rcx;
			rcx = rax;
			rcx >>= 0x5;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0xA;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x14;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x28;
			rax ^= rcx;
			rcx = Read<uint64_t>(rbp + 0xe8);
			rcx -= rsi;
			rcx = 0;
			rcx <<= 0x10;
			rcx ^= r10;
			_byteswap_uint64(rcx);
			rcx = Read<uint64_t>(rcx + 0x9);
			rax *= rcx;
			rcx = 0x1256A7FD88630E0C;
			rax -= rcx;
			rcx = (globals::module_base + 0x45A0);
			rcx = (~rcx);
			rcx *= rdi;
			rax ^= rcx;
			return rax;
			break;
		}

		case 6:
		{
			rsi = (globals::module_base + 0xC61);
			rbx = globals::module_base;
			r15 = (globals::module_base + 0xFA4E);
			r10 = Read<uint64_t>(globals::module_base + 0x6720149);
			rcx = rax;
			rcx >>= 0x26;
			rax ^= rcx;
			rcx = 0x910E74C8892A494B;
			rax *= rcx;
			rcx = 0x25BDAC759CD9A65A;
			rdx = Read<uint64_t>(rbp + 0xe8);
			rax ^= rcx;
			rdx -= rsi;
			rdx = 0;
			rdx <<= 0x10;
			rdx ^= r10;
			rcx = rax;
			rcx >>= 0x11;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x22;
			rcx ^= rax;
			rax = rdi;
			rax = (~rax);
			rcx += rax;
			_byteswap_uint64(rdx);
			rcx += r15;
			rax = Read<uint64_t>(rdx + 0x9);
			rax *= rcx;
			rcx = 0;
			rcx -= rdi;
			rcx -= rbx;
			rax += rcx;
			rcx = rax;
			rcx >>= 0x2;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x4;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x8;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x10;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x20;
			rax ^= rcx;
			return rax;
			break;
		}

		case 7:
		{
			r10 = Read<uint64_t>(globals::module_base + 0x6720149);
			rsi = (globals::module_base + 0xC61);
			rcx = rax;
			rcx >>= 0x23;
			rax ^= rcx;
			rax -= rdi;
			rcx = 0x73A4400838C7BC53;
			rax *= rcx;
			rcx = 0x6EB5E5469F1588A9;
			rax *= rcx;
			rax -= rdi;
			rcx = Read<uint64_t>(rbp + 0xe8);
			rcx -= rsi;
			rcx = 0;
			rcx <<= 0x10;
			rcx ^= r10;
			_byteswap_uint64(rcx);
			rax *= Read<uint64_t>(rcx + 0x9);
			rcx = 0x7FDE44377AC276FC;
			rax -= rcx;
			rax -= rdi;
			return rax;
			break;
		}

		case 8:
		{
			r10 = Read<uint64_t>(globals::module_base + 0x6720149);
			rsi = (globals::module_base + 0xC61);
			rbx = (globals::module_base + 0x516B2865);
			rcx = rdi;
			rcx *= rbx;
			rax -= rcx;
			rcx = rax;
			rcx >>= 0x28;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x19;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x32;
			rax ^= rcx;
			rcx = rax;
			rdx = Read<uint64_t>(rbp + 0xe8);
			rax = 0xBED5AFFE499C0152;
			rcx ^= rax;
			rdx -= rsi;
			rdx = 0;
			rdx <<= 0x10;
			rdx ^= r10;
			_byteswap_uint64(rdx);
			rax = Read<uint64_t>(rdx + 0x9);
			rax *= rcx;
			rcx = 0x807C3336097B0C37;
			rax *= rcx;
			rcx = rax;
			rcx >>= 0x1B;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x36;
			rax ^= rcx;
			rcx = 0xD93C7ACD831C4C60;
			rax ^= rcx;
			return rax;
			break;
		}

		case 9:
		{
			r10 = Read<uint64_t>(globals::module_base + 0x6720149);
			rsi = (globals::module_base + 0xC61);
			r11 = (globals::module_base + 0x576F9DC9);
			rcx = 0x68F6DF00F4BACC7A;
			rax ^= rcx;
			rcx = 0x56E7E66296809D1;
			rax += rcx;
			rcx = r11;
			rcx = (~rcx);
			rcx -= rdi;
			rax += rcx;
			rcx = 0xA12D9B2FBFD770E1;
			rax *= rcx;
			rcx = rax;
			rcx >>= 0x26;
			rcx ^= rdi;
			rax ^= rcx;
			rdx = Read<uint64_t>(rbp + 0xe8);
			rcx = rdi + rax * 1;
			rdx -= rsi;
			rax = (globals::module_base + 0x505E);
			rdx = 0;
			rcx += rax;
			rdx <<= 0x10;
			rdx ^= r10;
			_byteswap_uint64(rdx);
			rax = Read<uint64_t>(rdx + 0x9);
			rax *= rcx;
			return rax;
			break;
		}

		case 10:
		{
			r10 = Read<uint64_t>(globals::module_base + 0x6720149);
			rsi = (globals::module_base + 0xC61);
			rbx = (globals::module_base + 0xE836);
			rcx = 0x6EA059AB6BF1E7C9;
			rax += rcx;
			rdx = rdi;
			rdx = (~rdx);
			rax ^= rdi;
			rcx = rbx;
			rcx = (~rcx);
			rdx *= rcx;
			rax += rdx;
			rcx = rax;
			rcx >>= 0x9;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x12;
			rax ^= rcx;
			rdx = Read<uint64_t>(rbp + 0xe8);
			rdx -= rsi;
			rdx = 0;
			rcx = rax;
			rcx >>= 0x24;
			rcx ^= rax;
			rdx <<= 0x10;
			rdx ^= r10;
			_byteswap_uint64(rdx);
			rax = Read<uint64_t>(rdx + 0x9);
			rax *= rcx;
			rcx = 0x86BBE941893B7560;
			rax ^= rcx;
			rcx = 0x4B6AD93B6D408247;
			rax *= rcx;
			rcx = rax;
			rcx >>= 0x1C;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x38;
			rax ^= rcx;
			return rax;
			break;
		}

		case 11:
		{
			r10 = Read<uint64_t>(globals::module_base + 0x6720149);
			rsi = (globals::module_base + 0xC61);
			rbx = globals::module_base;
			rcx = rax;
			rcx >>= 0x12;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x24;
			rax ^= rcx;
			rcx = 0x7C3BEB8DC40811A8;
			rax -= rcx;
			rcx = rax;
			rcx >>= 0x1B;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x36;
			rax ^= rcx;
			rcx = rax;
			rdx = rbx + 0x4ebd4033;
			rcx >>= 0x1E;
			rdx += rdi;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x3C;
			rdx ^= rcx;
			rax ^= rdx;
			rcx = Read<uint64_t>(rbp + 0xe8);
			rcx -= rsi;
			rcx = 0;
			rcx <<= 0x10;
			rcx ^= r10;
			_byteswap_uint64(rcx);
			rcx = Read<uint64_t>(rcx + 0x9);
			rax *= rcx;
			rcx = 0x6076A3EF8DDAD3B9;
			rax *= rcx;
			rcx = 0xF986D49C2A28C6E;
			rax ^= rcx;
			return rax;
			break;
		}

		case 12:
		{
			rsi = (globals::module_base + 0xC61);
			rbx = globals::module_base;
			r11 = Read<uint64_t>(globals::module_base + 0x6720149);
			rcx = rax;
			rcx >>= 0x23;
			rcx ^= rax;
			rax = rdi + 0xffffffffffff33cd;
			rcx -= rbx;
			rax += rcx;
			rax -= rdi;
			rcx = 0x31D9D671DAFB458D;
			rax -= rcx;
			rcx = Read<uint64_t>(rbp + 0xe8);
			rcx -= rsi;
			rcx = 0;
			rcx <<= 0x10;
			rcx ^= r11;
			_byteswap_uint64(rcx);
			rax *= Read<uint64_t>(rcx + 0x9);
			rax -= rdi;
			rcx = 0x6DE4DD7F5DBEBF37;
			rax *= rcx;
			rcx = 0x10AD9BF11364611E;
			rax += rcx;
			return rax;
			break;
		}

		case 13:
		{
			r10 = Read<uint64_t>(globals::module_base + 0x6720149);
			rsi = (globals::module_base + 0xC61);
			r15 = (globals::module_base + 0x6BD9F7FF);
			rcx = r15;
			rcx -= rdi;
			rax ^= rcx;
			rcx = 0xB2E3E54D880FD59B;
			rax *= rcx;
			rcx = Read<uint64_t>(rbp + 0xe8);
			rcx -= rsi;
			rcx = 0;
			rcx <<= 0x10;
			rcx ^= r10;
			_byteswap_uint64(rcx);
			rax *= Read<uint64_t>(rcx + 0x9);
			rcx = 0xACF9B435578EDD81;
			rax *= rcx;
			rcx = rax;
			rcx >>= 0x1D;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x3A;
			rax ^= rcx;
			rcx = 0xA836451FE7F41082;
			rax -= rdi;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x6;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0xC;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x18;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x30;
			rax ^= rcx;
			return rax;
			break;
		}

		case 14:
		{
			r10 = Read<uint64_t>(globals::module_base + 0x6720149);
			rsi = (globals::module_base + 0xC61);
			rbx = globals::module_base;
			r15 = (globals::module_base + 0x453074BA);
			rcx = Read<uint64_t>(rbp + 0xe8);
			rdx = 0xBADA70100E8D2385;
			rcx -= rsi;
			rcx = 0;
			rcx <<= 0x10;
			rcx ^= r10;
			_byteswap_uint64(rcx);
			rcx = Read<uint64_t>(rcx + 0x9);
			rcx *= rdx;
			rax *= rcx;
			rcx = 0x224E473CDE2EFA81;
			rax += rcx;
			rdx = rax;
			rdx >>= 0x27;
			rdx ^= rax;
			rax = rdi;
			rax ^= r15;
			rax += rdx;
			rcx = rax;
			rcx >>= 0x12;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x24;
			rax ^= rcx;
			rcx = 0xD9D6BC80732609C3;
			rax *= rcx;
			rcx = rbx + 0xff12;
			rcx += rdi;
			rax += rcx;
			return rax;
			break;
		}

		case 15:
		{
			rsi = (globals::module_base + 0xC61);
			r15 = (globals::module_base + 0x17AD2D2E);
			r9 = Read<uint64_t>(globals::module_base + 0x6720149);
			rcx = rdi;
			rcx = (~rcx);
			rcx ^= r15;
			rax += rcx;
			rcx = Read<uint64_t>(rbp + 0xe8);
			rcx -= rsi;
			rcx = 0;
			rcx <<= 0x10;
			rcx ^= r9;
			_byteswap_uint64(rcx);
			rax *= Read<uint64_t>(rcx + 0x9);
			rcx = 0x8831BB2EDBA825E1;
			rax *= rcx;
			rcx = rax;
			rcx >>= 0x22;
			rax ^= rcx;
			rcx = (globals::module_base + 0x5AA25756);
			rcx = (~rcx);
			rax -= rdi;
			rax += rcx;
			rcx = rax;
			rcx >>= 0x14;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x28;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x8;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x10;
			rax ^= rcx;
			rcx = rax;
			rcx >>= 0x20;
			rax ^= rcx;
			rcx = 0xAAF6CD34DAA1E9A;
			rax += rcx;
			return rax;
			break;
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









