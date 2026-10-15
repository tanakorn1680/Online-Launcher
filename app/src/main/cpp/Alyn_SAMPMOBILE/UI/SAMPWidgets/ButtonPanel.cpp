#include "../UI.h"
#include "../../Features.h"
#include "../../AssetImageLoader.h"
#include "../../Game/Util.h"
#include "spdlog/spdlog.h"
#include "../../Game/Game.h"
#include "../../Net/NetGame.h"

/* ButtonPanel */

extern UI* pUI;
extern Game* pGame;
extern NetGame* pNetGame;

// Set by GuardButton, read by the CPad_GetEnterTargeting hook (Game/Pad.cpp).
extern bool g_bGuardButtonHeld;

// Round dark button with a white ring. Draws an icon PNG from the APK assets
// (app/src/main/assets/hud/*.png, same loader as StatusHUD); if the icon can't
// be loaded it falls back to a text label so the button is never invisible.
static void DrawRoundActionButton(ImGuiRenderer* renderer, const ImVec2& pos, const ImVec2& size, const std::string& label, void* icon, bool pressed)
{
	ImVec2 center = pos + size * 0.5f;
	float radius = (size.x < size.y ? size.x : size.y) * 0.5f;

	renderer->drawCircleFilled(center, radius, pressed ? ImColor(110, 110, 110, 215) : ImColor(0, 0, 0, 150));
	renderer->drawArc(center, radius - 2.0f, 4.0f, ImColor(255, 255, 255, 230), 0.0f, 360.0f);

	if (icon) {
		float iconSize = radius * 1.25f;
		ImVec2 half(iconSize * 0.5f, iconSize * 0.5f);
		renderer->drawImage(center - half, center + half, (ImTextureID) icon);
	}
	else {
		float fontSize = radius * 0.45f;
		ImVec2 textSize = renderer->calculateTextSize(label, fontSize);
		renderer->drawText(center - textSize * 0.5f, ImColor(255, 255, 255), label, true, fontSize);
	}
}

// Lazy load (render thread). LoadIconTextureFromAsset caches by path; we only
// retry a few times in case the AssetManager isn't registered yet.
static void* LoadActionIcon(const char* path, void*& cache, int& tries)
{
	if (!cache && tries < 5) {
		tries++;
		cache = LoadIconTextureFromAsset(path);
	}
	return cache;
}

ButtonPanel::ButtonPanel()
		: Layout(Orientation::HORIZONTAL)
{
	m_extended = false;

	m_extend = new Button(">>", UISettings::fontSize() / 2);
	m_esc = new Button("ESC", UISettings::fontSize() / 2);
	m_tab = new Button("TAB", UISettings::fontSize() / 2);
	m_alt = new Button("ALT", UISettings::fontSize() / 2);
	m_spc = new Button("SPC", UISettings::fontSize() / 2);
	m_f = new Button("F", UISettings::fontSize() / 2);
	m_h = new Button("H", UISettings::fontSize() / 2);
	m_2 = new Button("2", UISettings::fontSize() / 2);
	m_y = new Button("Y", UISettings::fontSize() / 2);
	m_n = new Button("N", UISettings::fontSize() / 2);
	m_c = new Button("C", UISettings::fontSize() / 2);
	//m_g = new Button("G", UISettings::fontSize() / 2);

	m_extend->setCallback([&]() {
		if (extended()) {
			minimize();
		}
		else {
			maximize();
		}
	});

	m_esc->setCallback([]() {
		if (pUI->scoreboard()->visible()) {
			pUI->scoreboard()->setVisible(false);
		}

		if (pNetGame) {
			if (pNetGame->GetTextDrawPool()) {
				pNetGame->GetTextDrawPool()->SetSelectState(false);
			}
		}
	});

	m_tab->setCallback([]() {
		if (!pUI->dialog()->visible() && !pUI->keyboard()->visible()) {
			pUI->scoreboard()->setVisible(!pUI->scoreboard()->visible());
		}

		if (!pGame->FindPlayerPed()->IsInVehicle()) {
			LocalPlayerKeys.bKeys[ePadKeys::KEY_ACTION] = true;
		}
	});

	m_alt->setCallback([]() {
		if (pGame->FindPlayerPed()->IsInVehicle()) {
			LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE] = true;
		}
		else {
			LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK] = true;
		}
	});

	m_spc->setCallback([]() {
		if (pGame->FindPlayerPed()->IsInVehicle()) {
			LocalPlayerKeys.bKeys[ePadKeys::KEY_HANDBRAKE] = true;
		}
		else {
			LocalPlayerKeys.bKeys[ePadKeys::KEY_SPRINT] = true;
		}
	});

	m_f->setCallback([]() {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK] = true;
	});

	m_h->setCallback([]() {
		if (pGame->FindPlayerPed()->IsInVehicle() && !pGame->FindPlayerPed()->IsAPassenger()) {
			LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH] = true;
		}
		else {
			LocalPlayerKeys.bKeys[ePadKeys::KEY_CTRL_BACK] = true;
		}
	});

	m_2->setCallback([]() {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_SUBMISSION] = true;
	});

	m_y->setCallback([]() {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_YES] = true;
	});

	m_n->setCallback([]() {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_NO] = true;
	});

	m_c->setCallback([]() {
		LocalPlayerKeys.bKeys[ePadKeys::KEY_CROUCH] = true;
	});

	/*m_g->setCallback([]() {
		if (pNetGame && pNetGame->GetPlayerPool() && pNetGame->GetPlayerPool()->GetLocalPlayer()) {
			pNetGame->GetPlayerPool()->GetLocalPlayer()->EnterVehicleAsPassenger();
		}
	});*/

	addChild(m_extend);
	addChild(m_esc);
	addChild(m_tab);
	addChild(m_alt);
	addChild(m_spc);
	addChild(m_f);
	addChild(m_h);
	addChild(m_2);
	addChild(m_y);
	addChild(m_n);
	addChild(m_c);
	//addChild(m_g);

	minimize();
}

void ButtonPanel::minimize()
{
	m_extended = false;

	m_extend->setVisible(true);
	m_extend->setCaption(">>");

	m_esc->setVisible(false);
	m_tab->setVisible(false);
	m_alt->setVisible(false);
	m_spc->setVisible(false);
	m_f->setVisible(false);
	m_h->setVisible(false);
	m_2->setVisible(false);
	m_y->setVisible(false);
	m_n->setVisible(false);
	m_c->setVisible(false);
	//m_g->setVisible(false);
}

void ButtonPanel::maximize()
{
	m_extended = true;

	m_extend->setVisible(true);
	m_extend->setCaption("<<");

	m_esc->setVisible(true);
	m_tab->setVisible(true);
	m_alt->setVisible(true);
	m_spc->setVisible(true);
	m_f->setVisible(true);
	m_h->setVisible(true);
	m_2->setVisible(true);
	m_y->setVisible(true);
	m_n->setVisible(true);
	m_c->setVisible(true);
	//m_g->setVisible(true);
}

/* PassengerButton */

ButtonPanel::PassengerButton::PassengerButton()
		: Image("samp", "gtexture")
{

}

void ButtonPanel::PassengerButton::draw(ImGuiRenderer* renderer)
{
	if (pNetGame) {
		CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
		CPlayerPed* pPlayerPed = pGame->FindPlayerPed();

		if (pVehiclePool && pPlayerPed && !pPlayerPed->IsInVehicle() && !pPlayerPed->IsAPassenger()) {
			VEHICLEID ClosetVehicleID = pVehiclePool->FindNearestToLocalPlayerPed();
			if (ClosetVehicleID < MAX_VEHICLES && pVehiclePool->GetSlotState(ClosetVehicleID)) {
				CVehicle* pVehicle = pVehiclePool->GetAt(ClosetVehicleID);
				if (pVehicle && pVehicle->GetDistanceFromLocalPlayerPed() < 4.0f) {
					static void* s_icon = nullptr;
					static int s_tries = 0;
					DrawRoundActionButton(renderer, absolutePosition(), size(), "RIDE", LoadActionIcon("hud/btn_door.png", s_icon, s_tries), focused());
				}
			}
		}
	}
}

void ButtonPanel::PassengerButton::touchPopEvent()
{
#if FEATURE_VEHICLE_COOLDOWN
	// Cooldown: spamming enter/exit can desync the ped task state and crash the game.
	static uint32_t s_lastRide = 0;
	uint32_t now = GetTickCount();
	if (s_lastRide != 0 && now - s_lastRide < 1000) {
		return;
	}
	s_lastRide = now;
#endif

	if (pNetGame && pNetGame->GetPlayerPool() && pNetGame->GetPlayerPool()->GetLocalPlayer()) {
		pNetGame->GetPlayerPool()->GetLocalPlayer()->EnterVehicleAsPassenger();
	}
}

/* GuardButton */

ButtonPanel::GuardButton::GuardButton()
{

}

void ButtonPanel::GuardButton::draw(ImGuiRenderer* renderer)
{
	bool show = false;

	if (pNetGame) {
		CPlayerPed* pPlayerPed = pGame->FindPlayerPed();
		show = pPlayerPed && !pPlayerPed->IsDead() && !pPlayerPed->IsInVehicle() && !pPlayerPed->IsAPassenger();
	}

	if (!show) {
		g_bGuardButtonHeld = false;
		return;
	}

	static void* s_icon = nullptr;
	static int s_tries = 0;
	DrawRoundActionButton(renderer, absolutePosition(), size(), "GUARD", LoadActionIcon("hud/btn_guard.png", s_icon, s_tries), focused());
}

void ButtonPanel::GuardButton::focuseEvent(bool focus)
{
	if (focus != g_bGuardButtonHeld) {
		spdlog::info("Guard button {}", focus ? "pressed" : "released");
	}
	g_bGuardButtonHeld = focus;
}
