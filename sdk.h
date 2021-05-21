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
	constexpr auto CLIENT_INFO = 0x170CB708;
	// 48 8B 83 ?? ?? ?? ?? C6 44 24 ?? ?? 0F B6
	constexpr auto CLIENT_BASE = 0x9DBD8;

	// 4C 8D 1D ? ? ? ? 44 8B 15 ? ? ? ? 48 8D 1D ? ? ? ? 4C 8B C9
	constexpr auto REFDEF = 0x170CE0A0;
	// 48 8B 05 ? ? ? ? 48 8B 7C 24 ? 48 05 ? ? ? ?
	constexpr auto CAMERA_BASE = 0x14324660;
	constexpr auto CAMERA_POS = 0x1D8;

	// C7 83 ? ? ? ? ? ? ? ? C7 83 ? ? ? ? ? ? ? ? E8 ? ? ? ? 44 0F B6 C6 48 8B D5 48 8B CF E8 ? ? ? ?
	constexpr auto PLAYER_DEAD_1 = 0x10;
	// 41 83 B8 ? ? ? ? ? 0F 85 ? ? ? ? 41 B8 ? ? ? ?
	constexpr auto PLAYER_DEAD_2 = 0x34;
	// 49 8B D9 41 0F B6 F0 8B F9 48 8B EA
	constexpr auto PLAYER_POS = 0x2D48;
	// 48 69 D3 ?? ?? ?? ?? 48 03 96 ?? ?? ?? ??
	constexpr auto PLAYER_SIZE = 0x3A90;
	// 8B 87 ? ? ? ? 4C 8B BC 24 ? ? ? ? 4C 8B B4 24 ? ? ? ? 4C 8B AC 24 ? ? ? ? 4C 8B A4 24 ? ? ? ? 85 C0 74 16
	constexpr auto PLAYER_TEAM = 0x17C;
	// C7 87 ?? ?? ?? ?? ?? ?? ?? ?? C7 87 ?? ?? ?? ?? ?? ?? ?? ?? 41
	constexpr auto PLAYER_VALID = 0x2C78;

	// 48 8D 0D ? ? ? ? 48 8B 0C C1 48 8B 01 FF 90 ? ? ? ?
	constexpr auto NAME_LIST = 0x14324660;
	constexpr auto NAME_OFFSET = 0x4C70;
	constexpr auto NAME_SIZE = 0xD0;
}
namespace decryption
{
	astatic uint64_t DecryptClientInfo() {
    static uint64_t rbp, rax, rcx, r8, rbx = 0;
    rbx= Read<uint64_t>(globals::module_base + offsets::CLIENT_INFO);
    if (!rbx)
        return 0;
 
    r8 = peb; //mov R8, gs:[RAX]
    rax = globals::module_base; //lea RAX, cs:7FF62B680000h
    //test rbx, rbx
    //jz short loc_7FF62D2FC359
    rcx = rbp + 0xF8; //mov RCX, [rbp + 0F8h]
    rbx += rax;
    rax = rbx;
    rax >>= 0x10;
    rbx ^= rax;
    rax = (globals::module_base + 0x134);
    rcx -= rax;
    rax = rbx;
    rcx = 0; //&= 0xffffffffc0000000 Special case;
    rax >>= 0x20;
    rax ^= rbx;
    rcx = _rotl64(rcx, 0x10); //RCX <<= 0x10;
    rcx ^= Read<uint64_t>(globals::module_base + 0x65DD10A);
    rcx = _byteswap_uint64(rcx);
    rbx = Read<uint64_t>(rcx + 0x13);
    rbx *= rax;
    rax = 0x76D048452DCF6909;
    rbx -= r8;
    r8 = (~r8);
    rbx *= rax;
    rax = (globals::module_base + 0x2C8CD073);
    r8 += rax;
    rbx ^= r8;
    return rbx;
}
 
uint64_t static DecryptClientBase(uint64_t client_info_ptr) {
 
    static uint64_t rax, rbx, rcx, rdx, rdi, rsi, rsp, rbp, r8, r9, r10, r11, r12, r13, r14, r15 = 0;
 
    if (!client_info_ptr)
        return 0;
 
    rax = Read<uint64_t>(client_info_ptr + offsets::CLIENT_INFO);
    if (!rax)
        return 0;
 
    rbx = peb;
    //test rax, rax
    //jz loc_7FF62D2FC078
    rcx = rbx;
    rcx <<= 0x21; //shl rcx, 21h
    rcx = _byteswap_uint64(rcx);
    // and ecx,0Fh
    rcx &= 0xF;
    switch (rcx)
    {
    case 0:
    {
        rdi = (globals::module_base + 0xACA);
        r9 = Read<uint64_t>(globals::module_base + 0x65DD140);
        //rcx = Read<uint64_t>(rbp + 0xe8);
        rcx -= rdi;
        rcx = 0; //&= 0xffffffffc0000000 Special case;
        rcx = _rotl64(rcx, 0x10); //rcx <<= 0x10;
        rcx ^= r9;
        rcx = _byteswap_uint64(rcx);
        rax *= Read<uint64_t>(rcx + 0xb);
        rcx = rax;
        rcx >>= 0x18;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x30;
        rax ^= rcx;
        rcx = globals::module_base;
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
        rcx = 0x345963FE4F9F5BC7;
        rax *= rcx;
        rcx = 0x1BC0D0E9288C6DB3;
        rax += rcx;
        rax += rbx;
        rcx = globals::module_base;
        rax -= rcx;
        return rax;
    }
    case 1:
    {
        rdi = (globals::module_base + 0xACA);
        r11 = (globals::module_base + 0x6064722A);
        r10 = Read<uint64_t>(globals::module_base + 0x65DD140);
        rcx = rbx;
        rcx ^= r11;
        rax -= rcx;
        rcx = rax;
        rcx >>= 0x11;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x22;
        rax ^= rcx;
        rax ^= rbx;
        rcx = (globals::module_base + 0x28AB);
        rcx = (~rcx);
        rcx -= rbx;
        rax += rcx;
        rcx = 0x16A1C31B3D93A83F;
        rax *= rcx;
        rcx = 0xD0C234BF8A55764B;
        rax *= rcx;
        //rcx = Read<uint64_t>(rbp + 0xe8);
        rcx -= rdi;
        rcx = 0; //&= 0xffffffffc0000000 Special case;
        rcx = _rotl64(rcx, 0x10); //rcx <<= 0x10;
        rcx ^= r10;
        rcx = _byteswap_uint64(rcx);
        rax *= Read<uint64_t>(rcx + 0xb);
        rcx = 0xB75E6F62B4DBBCC1;
        rax *= rcx;
        return rax;
    }
    case 2:
    {
        rdi = (globals::module_base + 0xACA);
        r14 = (globals::module_base + 0x30A5);
        r10 = Read<uint64_t>(globals::module_base + 0x65DD140);
        rcx = rax;
        rcx >>= 0x15;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x2A;
        rax ^= rcx;
        rcx = rbx;
        rcx = (~rcx);
        rcx ^= r14;
        rax -= rcx;
        //rcx = Read<uint64_t>(rbp + 0xe8);
        rcx -= rdi;
        rcx = 0; //&= 0xffffffffc0000000 Special case;
        rcx = _rotl64(rcx, 0x10); //rcx <<= 0x10;
        rcx ^= r10;
        rcx = _byteswap_uint64(rcx);
        rax *= Read<uint64_t>(rcx + 0xb);
        rcx = 0x5D11A30DE94FFEDE;
        rax += rcx;
        rcx = rax;
        rcx >>= 0x1B;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x36;
        rax ^= rcx;
        rax ^= rbx;
        rcx = 0x1D2CA89A1A1BE3D9;
        rax ^= rcx;
        rcx = 0xDD63D27B22050957;
        rax *= rcx;
        return rax;
    }
    case 3:
    {
       rdi = (globals::module_base + 0xACA);
        r14 = (globals::module_base + 0x7B3CDBC1);
        r10 = Read<uint64_t>(globals::module_base + 0x65DD140);
        rdx = rbx;
        rdx = (~rdx);
        rcx = r14;
        rcx = (~rcx);
        rdx *= rcx;
        rax += rdx;
        rcx = rax;
        rcx >>= 0x26;
        rcx ^= rax;
        rax = rcx + rbx * 2;
        rcx = globals::module_base;
        rax -= rcx;
        rax -= 0x7736E4C5;
        rcx = 0xA4C7B3171334DA2E;
        rax ^= rcx;
        rcx = 0x667B75570F23711D;
        rax *= rcx;
        rcx = 0x7E05078E8B5B3EDA;
        rax -= rcx;
        //rcx = Read<uint64_t>(rbp + 0xe8);
        rcx -= rdi;
        rcx = 0; //&= 0xffffffffc0000000 Special case;
        rcx = _rotl64(rcx, 0x10); //rcx <<= 0x10;
        rcx ^= r10;
        rcx = _byteswap_uint64(rcx);
        rax *= Read<uint64_t>(rcx + 0xb);
        return rax;
    }
    case 4:
    {
        rdi = (globals::module_base + 0xACA);
        r9 = Read<uint64_t>(globals::module_base + 0x65DD140);
        rax ^= rbx;
        //rcx = Read<uint64_t>(rbp + 0xe8);
        rcx -= rdi;
        rcx = 0; //&= 0xffffffffc0000000 Special case;
        rcx = _rotl64(rcx, 0x10); //rcx <<= 0x10;
        rcx ^= r9;
        rcx = _byteswap_uint64(rcx);
        rcx = Read<uint64_t>(rcx + 0xb);
        rcx *= 0x64DE26759A457153;
        rax *= rcx;
        rcx = rax;
        rcx >>= 0x24;
        rax ^= rcx;
        rcx = 0x49AF5B2E74070925;
        rax *= rcx;
        rcx = 0xB5CC279242DD0301;
        rax *= rcx;
        return rax;
    }
    case 5:
    {
        r11 = Read<uint64_t>(globals::module_base + 0x65DD140);
        rdi = (globals::module_base + 0xACA);
        r15 = (globals::module_base + 0x6BA9);
        rdx = (globals::module_base + 0x5F9E55C9);
        rdx = (~rdx);
        rdx ^= rbx;
        rcx = rax;
        rax = 0xBF5978C960F6BB4B;
        rax ^= rcx;
        rax += rdx;
        rdx = (globals::module_base + 0x28877536);
        rcx = rax;
        rcx >>= 0x18;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x30;
        rax ^= rcx;
        rcx = rbx;
        rcx = (~rcx);
        rcx *= r15;
        rax ^= rcx;
        //r8 = Read<uint64_t>(rbp + 0xe8);
        r8 -= rdi;
        r8 = 0; //&= 0xffffffffc0000000 Special case;
        r8 = _rotl64(r8, 0x10); //r8 <<= 0x10;
        r8 ^= r11;
        rcx = rbx;
        rcx *= rdx;
        rdx = rax;
        rdx -= rcx;
        rcx = 0x84229F2B4FE6843B;
        r8 = _byteswap_uint64(r8);
        rax = Read<uint64_t>(r8 + 0xb);
        rax *= rdx;
        rax *= rcx;
        rax ^= rbx;
        return rax;
    }
    case 6:
    {
        rdi = (globals::module_base + 0xACA);
        r15 = (globals::module_base + 0xE397);
        r10 = Read<uint64_t>(globals::module_base + 0x65DD140);
        rcx = globals::module_base;
        rax += rcx;
        rcx = globals::module_base;
        rax += rcx;
        rcx = 0x4030351D523D85BB;
        rax += rcx;
        rcx = rbx;
        rcx ^= r15;
        rax += rcx;
        rcx = 0x71A01F36E5BF55AF;
        rax *= rcx;
        rcx = rax;
        rcx >>= 0x10;
        rax ^= rcx;
        rcx = rax;
        //rdx = Read<uint64_t>(rbp + 0xe8);
        rdx -= rdi;
        rcx >>= 0x20;
        rcx ^= rax;
        rdx = 0; //&= 0xffffffffc0000000 Special case;
        rdx = _rotl64(rdx, 0x10); //rdx <<= 0x10;
        rdx ^= r10;
        rdx = _byteswap_uint64(rdx);
        rax = Read<uint64_t>(rdx + 0xb);
        rdx = (globals::module_base + 0x31AFF9CE);
        rax *= rcx;
        rcx = rbx;
        rcx *= rdx;
        rax -= rcx;
        return rax;
    }
    case 7:
    {
        r10 = Read<uint64_t>(globals::module_base + 0x65DD140);
        rdi = (globals::module_base + 0xACA);
        r15 = (globals::module_base + 0x9CF0);
        rcx = rax;
        rcx >>= 0x19;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x32;
        rax ^= rcx;
        rcx = globals::module_base;
        rcx += 0x16E9;
        rcx += rbx;
        rax ^= rcx;
        //rdx = Read<uint64_t>(rbp + 0xe8);
        rdx -= rdi;
        rdx = 0;
        rdx = _rotl64(rdx, 0x10); //rdx <<= 0x10;
        rdx ^= r10;
        rcx = rbx;
        rdx = _byteswap_uint64(rdx);
        rcx ^= r15;
        rdx = Read<uint64_t>(rdx + 0xb);
        rax *= rdx;
        rax -= rcx;
        rcx = rax;
        rcx >>= 0xA;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x14;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x28;
        rax ^= rcx;
        rcx = 0x201300BD919020EB;
        rax *= rcx;
        rcx = 0x136871F8B2311042;
        rax += rcx;
        rcx = 0xE0229051A9F3C38B;
        rax ^= rcx;
        return rax;
    }
    case 8:
    {
        rdi = (globals::module_base + 0xACA);
        r15 = (globals::module_base + 0x6C04);
        r10 = Read<uint64_t>(globals::module_base + 0x65DD140);
        //rcx = Read<uint64_t>(rbp + 0xe8);
        rcx -= rdi;
        rcx = 0; //&= 0xffffffffc0000000 Special case;
        rcx = _rotl64(rcx, 0x10); //rcx <<= 0x10;
        rcx ^= r10;
        rcx = _byteswap_uint64(rcx);
        rdx = Read<uint64_t>(rcx + 0xb);
        rcx = 0x866F75E98D0D53B1;
        rdx *= rax;
        rax = rbx;
        rax *= r15;
        rdx += rax;
        rax = 0x1671E2558441F0BB;
        rdx ^= rbx;
        rax = rdx;
        rax >>= 0x20;
        rax ^= rdx;
        rax ^= rcx;
        rcx = 0x9E0D951F0C28F90B;
        rax *= rcx;
        rcx = 0x78503CB374B04FAD;
        rax *= rcx;
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
    }
    case 9:
    {
        rdi = (globals::module_base + 0xACA);
        r14 = (globals::module_base + 0x6CFB74E0);
        r11 = (globals::module_base + 0x7F309832);
        r9 = Read<uint64_t>(globals::module_base + 0x65DD140);
        //rcx = Read<uint64_t>(rbp + 0xe8);
        rcx -= rdi;
        rcx = 0; //&= 0xffffffffc0000000 Special case;
        rcx = _rotl64(rcx, 0x10); //rcx <<= 0x10;
        rcx ^= r9;
        rcx = _byteswap_uint64(rcx);
        rax *= Read<uint64_t>(rcx + 0xb);
        rax ^= rbx;
        rax ^= r14;
        rcx = rax;
        rcx >>= 0x17;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x2E;
        rax ^= rcx;
        rcx = 0xD7356E290A5B1FBA;
        rax += rcx;
        rcx = globals::module_base;
        rax ^= rcx;
        rcx = 0xD80D8A31210F08D3;
        rax *= rcx;
        rcx = r11;
        rcx = (~rcx);
        rcx ^= rbx;
        rax -= rcx;
        rcx = rax;
        rcx >>= 0x9;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x12;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x24;
        rax ^= rcx;
        return rax;
    }
    case 10:
    {
        r9 = Read<uint64_t>(globals::module_base + 0x65DD140);
        rdi = (globals::module_base + 0xACA);
        r11 = (globals::module_base + 0x6AD2A7C4);
        rax -= rbx;
        rax ^= rbx;
        rcx = 0x29222BE3E0E2FFB;
        rax ^= r11;
        r11 = globals::module_base;
        rax *= rcx;
        rcx = 0x5BB04B85CD9365D;
        rax -= rbx;
        rax += rcx;
        rax += r11;
        //rcx = Read<uint64_t>(rbp + 0xe8);
        rcx -= rdi;
        rcx = 0; //&= 0xffffffffc0000000 Special case;
        rcx = _rotl64(rcx, 0x10); //rcx <<= 0x10;
        rcx ^= r9;
        rcx = _byteswap_uint64(rcx);
        rax *= Read<uint64_t>(rcx + 0xb);
        rcx = 0x5FC588EC700475F3;
        rax *= rcx;
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
    }
    case 11:
    {
        r10 = Read<uint64_t>(globals::module_base + 0x65DD140);
        rdi = (globals::module_base + 0xACA);
        r14 = (globals::module_base + 0xCF97);
        rdx = r14;
        rdx = (~rdx);
        rdx += rbx;
        rax ^= rdx;
        rcx = (globals::module_base + 0xCA22);
        rax += rbx;
        rdx = globals::module_base;
        rax += rcx;
        rcx = rbx;
        rcx = (~rcx);
        rcx -= rdx;
        rcx -= 0x1236;
        rax ^= rcx;
        rcx = 0x48502E6384BA9941;
        rax *= rcx;
        rcx = 0x5EB925E16D423E1E;
        rax -= rcx;
        //rcx = Read<uint64_t>(rbp + 0xe8);
        rcx -= rdi;
        rcx = 0; //&= 0xffffffffc0000000 Special case;
        rcx = _rotl64(rcx, 0x10); //rcx <<= 0x10;
        rcx ^= r10;
        rcx = _byteswap_uint64(rcx);
        rax *= Read<uint64_t>(rcx + 0xb);
        rcx = 0xE5AB625D3BB65BBF;
        rax *= rcx;
        rcx = rax;
        rcx >>= 0x1F;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x3E;
        rax ^= rcx;
        return rax;
    }
    case 12:
    {
        rdi = (globals::module_base + 0xACA);
        r15 = (globals::module_base + 0xEE34);
        r10 = Read<uint64_t>(globals::module_base + 0x65DD140);
        rcx = rbx + 0x1;
        rcx *= r15;
        rax += rcx;
        rax ^= rbx;
        rcx = 0xBF0F6EC504339C71;
        rax *= rcx;
        rcx = 0x62753D45ABF968CD;
        rax -= rcx;
        rcx = 0x28C82E52D21EB6AB;
        rax -= rcx;
        //rcx = Read<uint64_t>(rbp + 0xe8);
        rcx -= rdi;
        rcx = 0; //&= 0xffffffffc0000000 Special case;
        rcx = _rotl64(rcx, 0x10); //rcx <<= 0x10;
        rcx ^= r10;
        rcx = _byteswap_uint64(rcx);
        rax *= Read<uint64_t>(rcx + 0xb);
        rcx = rax;
        rcx >>= 0xB;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x16;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x2C;
        rax ^= rcx;
        rcx = globals::module_base;
        rax ^= rcx;
        return rax;
    }
    case 13:
    {
        r10 = Read<uint64_t>(globals::module_base + 0x65DD140);
        rdi = (globals::module_base + 0xACA);
        //rcx = Read<uint64_t>(rbp + 0xe8);
        rcx -= rdi;
        rcx = 0; //&= 0xffffffffc0000000 Special case; //&= 0xffffffffc0000000 Special case
        rcx = _rotl64(rcx, 0x10); //rcx <<= 0x10;
        rcx ^= r10;
        rcx = _byteswap_uint64(rcx);
        rax *= Read<uint64_t>(rcx + 0xb);
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
        rdx = rax;
        rdx >>= 0x22;
        rdx ^= rax;
        rcx = 0xAB96BD5255F50EEF;
        rax = (globals::module_base + 0x4795B778);
        rax = (~rax);
        rax ^= rbx;
        rax += rdx;
        rax *= rcx;
        rax -= rbx;
        rcx = 0x697DECF064AB09C3;
        rax *= rcx;
        rcx = rbx;
        rcx *= 0x7FF7C6D3E842;
        rax += rcx;
       return rax;
    }
    case 14:
    {
        rdi = (globals::module_base + 0xACA);
        r9 = Read<uint64_t>(globals::module_base + 0x65DD140);
        rcx = rax;
        rcx >>= 0xB;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x16;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x2C;
        rax ^= rcx;
        //rcx = Read<uint64_t>(rbp + 0xe8);
        rcx -= rdi;
        rcx = 0; //&= 0xffffffffc0000000 Special case; //&= 0xffffffffc0000000 Special case
        rcx = _rotl64(rcx, 0x10); //rcx <<= 0x10;
        rcx ^= r9;
        rcx = _byteswap_uint64(rcx);
        rcx = Read<uint64_t>(rcx + 0xb);
        rcx *= 0xF2B84228009F892B;
        rax *= rcx;
        r10 = 0x21D0F0E2660F5094;
        rcx = rbx;
        rcx = (~rcx);
        rcx *= 0x7FF7C6D32E00;
        rcx += r10;
        rax += rcx;
        rcx = rax;
        rcx >>= 0x10;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x20;
        rax ^= rcx;
        rcx = 0x1E450D45A88B3DC9;
        rax *= rcx;
        rcx = rax;
        rcx >>= 0x17;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x2E;
        rax ^= rcx;
        return rax;
    }
    case 15:
    {
        rdi = (globals::module_base + 0xACA);
        r9 = Read<uint64_t>(globals::module_base + 0x65DD140);
        rcx = globals::module_base;
        rax ^= rcx;
        rcx = 0x104FF8B4C43406AD;
        rax += rcx;
        rcx = 0x16DB4431461A3E29;
        rax *= rcx;
        //rcx = Read<uint64_t>(rbp + 0xe8);
        rcx -= rdi;
        rcx = 0; //&= 0xffffffffc0000000 Special case; //&= 0xffffffffc0000000 Special case
        rcx = _rotl64(rcx, 0x10); //rcx <<= 0x10;
        rcx ^= r9;
        rcx = _byteswap_uint64(rcx);
        rax *= Read<uint64_t>(rcx + 0xb);
        rcx = rax;
        rcx >>= 0x13;
        rax ^= rcx;
        rcx = rax;
        rcx >>= 0x26;
        rax ^= rcx;
        rcx = globals::module_base;
        rax -= rcx;
        rax += 0xFFFFFFFFFFFF9A85;
        rax += rbx;
        rcx = 0x11B2D7215841BEB4;
        rax += rcx;
        return rax;
    }
    }
    return 0;
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









