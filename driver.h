#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <vector>
#include <memoryapi.h>
#include <string>
#include <chrono>
#include <iostream>
#include "xor.h"

#define CODE_READ				0xA1;
#define CODE_GET_PEB		0xA2;
#define CODE_GET_BASE		0xA3;
#define CODE_SUCCESS		0x0;
#define CODE_FAILURE		0x1;
typedef struct _COMMS
{
	UINT32 code;							// Determines what action to take
	ULONG pid;								// ID of target process
	ULONG64 addy;							// Field used to communicate addresses
	void* buff;								// Pointer to output buffer
	ULONGLONG size;						// Size of output
	const char* str;					// General purpose string field

} COMMS;

static std::uint32_t process_id = 0;
static COMMS m = { 0 };

struct HandleDisposer
{
	using pointer = HANDLE;
	void operator()(HANDLE handle) const
	{
		if (handle != NULL || handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(handle);
		}
	}
};

using unique_handle = std::unique_ptr<HANDLE, HandleDisposer>;

static std::uint32_t get_process_id(std::string_view process_name) {
	PROCESSENTRY32 processentry;
	const unique_handle snapshot_handle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));

	if (snapshot_handle.get() == INVALID_HANDLE_VALUE)
		return 0;

	processentry.dwSize = sizeof(MODULEENTRY32);

	while (Process32Next(snapshot_handle.get(), &processentry) == TRUE) {
		if (process_name.compare(processentry.szExeFile) == 0)
			return processentry.th32ProcessID;
	}
	return 0;
}

template<typename ... A>
uint64_t call_hook(const A ... arguments)
{
	void* control_function = GetProcAddress(LoadLibrary("win32u.dll"), xorstr_("NtOpenCompositionSurfaceSectionInfo"));

	const auto control = static_cast<uint64_t(__stdcall*)(A...)>(control_function);

	return control(arguments ...);
}


static ULONG64 DriverGetBase(const char* modName) {
	m.code = CODE_GET_BASE;
	m.pid = process_id;
	m.str = modName;

	call_hook(&m);
	m.str = nullptr;

	return m.addy;
}

static ULONG64 DriverGetPEB() {
	m.code = CODE_GET_PEB;
	m.pid = process_id;

	call_hook(&m);
	return m.addy;
}

template <typename T>
T Read(UINT_PTR TargetAddress) {

	m.code = CODE_READ;
	m.addy = TargetAddress;
	m.size = sizeof(T);
	T output;
	m.buff = &output;

	call_hook(&m);

	return output;
}




