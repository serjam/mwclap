#include "sdk.h"
#include "game.h"
#include "xor.h"
#include <thread>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hSecInstance, LPSTR nCmdLine, INT nCmdShow)
{
	LoadLibrary("user32.dll");

#if DEBUG
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
#endif
	
	char* gn = globals::decGn();

	while (!process_id) {
		process_id = get_process_id(gn);
#if DEBUG
		utils::log(xorstr_("[+] Waiting for game to open..."));
#endif
		Sleep(1000);
	}
	globals::module_base = DriverGetBase(gn);

	memset(gn, 0, 18);
	delete[] gn;

	globals::peb = DriverGetPEB();

#if DEBUG
	utils::log(xorstr_("[+] Found Game!"));
	utils::loghex(xorstr_("[>] PEB: "), globals::peb);
	utils::loghex(xorstr_("[>] BASE: "), globals::module_base);
#endif

	Game& g_MW = Game::get();
	g_MW.Init();

	while (1) {
#if DEBUG
		if (GetAsyncKeyState(VK_PRIOR) & 1) {
			g_MW.UpdateAddresses();
		}
		if (GetAsyncKeyState(VK_NEXT) & 1) {
			g_MW.PrintEntityDbg();
		}
#endif
		if (GetAsyncKeyState(VK_HOME) & 1) {
			g_MW.ToggleESP();
		}
		if (GetAsyncKeyState(VK_END) & 1) {
			g_MW.End();
			break;
		}
	}

#if DEBUG
	fclose(f);
	FreeConsole();
#endif

}
