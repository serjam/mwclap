#pragma once
#include "sdk.h"
#include "player.h"
#include "overlay.h"
#include <thread>

class Game {
private:
	Game() {}
	FOverlay* overlay;
	std::thread olayT;
	std::thread addrT;

public:
	bool espOn;
	bool terminate;

	Game(Game const&) = delete;
	Game& operator=(Game const&) = delete;

	static Game& get() {
		static Game mw;
		return mw;
	}

	static void RenderOlay(FOverlay* overlay)
	{
		Game& g_MW = Game::get();
		while (!g_MW.terminate)
		{
			overlay->begin_scene();
			overlay->clear_scene();
			overlay->draw_text_green(10, 20, xorstr_("GLARE HAX v1.0"));
			g_MW.DrawESP();
			overlay->end_scene();
		}
		overlay->clear_screen();
	}
	static void UpdateAddresses() {
		Game& g_MW = Game::get();
		while (!g_MW.terminate) {
			globals::INFO = GetClientInfo();
			globals::BASE = GetClientBase(globals::INFO);
			globals::REFDEF = GetRefDef();
			globals::NAMES = GetNameList();
#if DEBUG
			utils::loghex(xorstr_("[*] Dec Info Ptr: "), globals::INFO);
			utils::loghex(xorstr_("[*] Dec Base Ptr: "), globals::BASE);
			utils::loghex(xorstr_("[*] Dec Refdef Ptr: "), globals::REFDEF);
			utils::loghex(xorstr_("[*] Name List Ptr: "), globals::NAMES);
#endif
			Sleep(15000);
		}
	}

	void Init() {
		overlay = { 0 };
		espOn = false;
		terminate = false;

		addrT = std::thread(UpdateAddresses);

		InitOlay();
	}

	void End() {
		terminate = true;
		olayT.join();
		addrT.join();
		overlay->clear_screen();
		overlay->d2d_shutdown();
	}

	void InitOlay()
	{
		if (!overlay->window_init())
			return;
		if (!overlay->init_d2d())
			return;

		olayT = std::thread(RenderOlay, overlay);

		return;
	}

	Vector3 GetLocalPos()
	{
		uint64_t cptr = Read<uint64_t>(globals::module_base + offsets::CAMERA_BASE);
		return Read<Vector3>(cptr + offsets::CAMERA_POS);
	}

	bool W2S(Vector3 worldLocation, Vector3 cameraPosition,
		int screenWidth, int screenHeight,
		Vector2 fieldOfView, Vector3* matrices, Vector2& out)
	{
		Vector3 local = worldLocation - cameraPosition;

		Vector3 trans = Vector3(local.Dot(matrices[1]), local.Dot(matrices[2]), local.Dot(matrices[0]));

		if (trans.z < 0.01f)
			return false;

		out.x = (((float)screenWidth / 2) * (1 - (trans.x / fieldOfView.x / trans.z)));
		out.y = (((float)screenHeight / 2) * (1 - (trans.y / fieldOfView.y / trans.z)));
		
		if (out.x > screenWidth || out.x < 0 || out.y > screenHeight || out.y < 0)
			return false;

		return true;
	}

	void ToggleESP() { espOn = !espOn; }

	void DrawESP() {
		if (!espOn || !globals::BASE || !globals::NAMES) return;

		Vector3 localPos = GetLocalPos();
		auto rd = Read<refdef_t>(globals::REFDEF);
		Vector2 screenFPos;
		Vector2 screenHPos;

		for (int i = 0; i < 155; i++)
		{
			Player* p = Player::GetPlayerAt(i);
			NameEntry ne = p->getNameEntry();

			if (!p || !p->isValid() || ne.health <= 0)
				continue;

			Vector3 feetPos = p->getOrigin();
			Vector3 headPos = p->getOrigin() + Vector3(0.0, 0.0, 58.0);

			if (W2S(feetPos, localPos, rd.width, rd.height, 
								rd.view.tanHalfFov, rd.view.axis, screenFPos) &&
					W2S(headPos, localPos, rd.width, rd.height,
								rd.view.tanHalfFov, rd.view.axis, screenHPos)) {
				float boxHeight = screenFPos.y - screenHPos.y;
				float boxWidth = boxHeight / 4.0;
				overlay->draw_box(screenHPos.x - boxWidth, screenHPos.y, boxWidth, boxHeight);
				overlay->draw_text_white(10.0, screenFPos.x, screenFPos.y, ne.name);
			}
		}
	}

#if DEBUG
	void PrintEntityDbg() {
		if (!globals::BASE) return;

		Vector3 localPos = GetLocalPos();
		auto rd = Read<refdef_t>(globals::REFDEF);
		Vector2 spos;

		for (int i = 0; i < 155; i++)
		{
			Player* p = Player::GetPlayerAt(i);
			NameEntry ne = p->getNameEntry();

			if (!p || !p->isValid() || ne.health <= 0)
				continue;

			Vector3 pos = p->getOrigin();
			printf(xorstr_("P# %d :: X = %f :: Y = %f :: Z = %f\n"), i, pos.x, pos.y, pos.z);
			printf(xorstr_("NAME = %s :: HEALTH = %d\n"), ne.name, ne.health);
			if (W2S(pos, localPos, rd.width, rd.height, rd.view.tanHalfFov, rd.view.axis, spos)) {
				printf(xorstr_("ESP :: %f, %f\n"), spos.x, spos.y);
			}
			printf("\n");
		}
	}
#endif

};


