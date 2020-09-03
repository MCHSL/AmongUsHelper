// dllmain.cpp : Defines the entry point for the DLL application.

#define WIN32_LEAN_AND_MEAN 
#include <map>
#include <memory>
#include <string>
#include <windows.h>
#include "sigscan/sigscan.h"
#include "subhook/subhook.h"
#include "kiero.h"
#include "imgui_group_panel.hpp"

#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>


#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = NULL;

std::map<void*, std::unique_ptr<subhook::Hook>> hooks;

struct __declspec(align(4)) ClientData__Fields
{
    int32_t Id;
    bool InScene;
    bool IsReady;
    void* Character;
};

struct ClientData
{
    void* klass;
    void* monitor;
    struct ClientData__Fields fields;
};

struct PlayerControl__Fields
{
    void* _;
    int32_t LastStartCounter;
    uint8_t PlayerId;
    /*float MaxReportDistance;
    bool moveable;
    bool inVent;
    struct GameData_PlayerInfo* _cachedData;
    struct AudioSource* FootSteps;
    struct AudioClip* KillSfx;
    struct KillAnimation__Array* KillAnimations;
    float killTimer;
    int32_t RemainingEmergencies;
    struct TextRenderer* nameText;
    struct LightSource* LightPrefab;
    struct LightSource* myLight;
    struct Collider2D* Collider;
    struct PlayerPhysics* MyPhysics;
    struct CustomNetworkTransform* NetTransform;
    struct PetBehaviour* CurrentPet;
    struct HatParent* HatRenderer;
    struct SpriteRenderer* myRend;
    struct Collider2D__Array* hitBuffer;
    struct List_1_PlayerTask_* myTasks;
    struct SpriteAnim__Array* ScannerAnims;
    struct SpriteRenderer__Array* ScannersImages;
    struct IUsable* closest;
    bool isNew;
    struct Dictionary_2_UnityEngine_Collider2D_IUsable_* cache;
    struct List_1_IUsable_* itemsInRange;
    struct List_1_IUsable_* newItemsInRange;
    uint8_t scannerCount;
    bool infectedSet;*/
};

struct PlayerControl
{
    void* klass;
    void* monitor;
    struct PlayerControl__Fields fields;
};

struct __declspec(align(4)) GameData_PlayerInfo__Fields
{
    uint8_t PlayerId;
    void* PlayerName;
    uint8_t ColorId;
    uint32_t HatId;
    uint32_t PetId;
    uint32_t SkinId;
    bool Disconnected;
    struct List_1_GameData_TaskInfo_* Tasks;
    bool IsImpostor;
    bool IsDead;
    struct PlayerControl* _object;
};

struct GameData_PlayerInfo
{
    void* klass;
    void* monitor;
    struct GameData_PlayerInfo__Fields fields;
};

typedef void(*SetPlayerColorPtr)(void* something, unsigned char id, unsigned char color);
typedef void(*OnPlayerDisconnectPtr)(void* something, ClientData* cdata, unsigned int reason);
typedef GameData_PlayerInfo*(*GetPlayerInfoByIdPtr)(void* GameData, unsigned char id);
typedef void(*MurderPlayerPtr)(PlayerControl* me, PlayerControl* target);
typedef void(*RevivePtr)(PlayerControl* _this);
typedef PlayerControl* (*GetPlayerControl)(GameData_PlayerInfo* _this);

void* GameData;

SetPlayerColorPtr oSetPlayerColor;
OnPlayerDisconnectPtr oOnPlayerDisconnect;
GetPlayerInfoByIdPtr oGetPlayerInfoById;
MurderPlayerPtr oMurderPlayer;
RevivePtr oRevive;

void* untyped_install_hook(void* original, void* hook)
{
    std::unique_ptr<subhook::Hook> /*I am*/ shook = std::make_unique<subhook::Hook>();
    shook->Install(original, hook);
    const auto* trampoline = shook->GetTrampoline();
    hooks[original] = std::move(shook);
    return (void*)trampoline;
}

enum class Color : unsigned char
{
	RED,
	BLUE,
	GREEN,
	PINK,
	ORANGE,
	YELLOW,
	BLACK,
	WHITE,
	PURPLE,
	BROWN,
	CYAN,
	LIME
};


enum TrustStatus
{
    TRUSTED,
    PLAUSIBLE,
    NEUTRAL,
    SUSPICIOUS,
    IMPOSTOR,
};

struct PlayerInfo
{
    Color color;
    TrustStatus trust = NEUTRAL;
};

std::map<unsigned char, PlayerInfo> players;

const char* cid2text(Color color_id)
{
	switch(color_id)
	{
    case Color::RED:
        return "RED";
    case Color::BLUE:
        return "BLUE";
    case Color::GREEN:
        return "GREEN";
    case Color::PINK:
        return "PINK";
    case Color::ORANGE:
        return "ORANGE";
    case Color::YELLOW:
        return "YELLOW";
    case Color::BLACK:
        return "BLACK";
    case Color::WHITE:
        return "WHITE";
    case Color::PURPLE:
        return "PURPLE";
    case Color::BROWN:
        return "BROWN";
    case Color::CYAN:
        return "CYAN";
    case Color::LIME:
        return "LIME";
    default:
        return "UNKNOWN";
	}
}

ImVec4 cid2color(Color cid)
{
    switch (cid)
    {
    case Color::RED:
        return {0xc6/255.0, 0x11/255.0, 0x11/255.0, 0.45};
    case Color::BLUE:
        return { 0x13 / 255.0, 0x2e / 255.0, 0xd2 / 255.0, 0.45 };;
    case Color::GREEN:
        return { 0x11 / 255.0, 0x80 / 255.0, 0x2d / 255.0, 0.45 };;
    case Color::PINK:
        return { 0xee / 255.0, 0x54 / 255.0, 0xbb / 255.0, 0.45 };;
    case Color::ORANGE:
        return { 0xf0 / 255.0, 0x7d / 255.0, 0x0d / 255.0, 0.45 };;
    case Color::YELLOW:
        return { 0xf6 / 255.0, 0xf6 / 255.0, 0x57 / 255.0, 0.45 };;
    case Color::BLACK:
        return { 0x3f / 255.0, 0x47 / 255.0, 0x4e / 255.0, 0.45 };;
    case Color::WHITE:
        return { 0xd7 / 255.0, 0xe1 / 255.0, 0xf1 / 255.0, 0.45 };;
    case Color::PURPLE:
        return { 0x6b / 255.0, 0x2f / 255.0, 0xbc / 255.0, 0.45 };;
    case Color::BROWN:
        return { 0x79 / 255.0, 0x49 / 255.0, 0x1e / 255.0, 0.45 };;
    case Color::CYAN:
        return { 0x38 / 255.0, 0xff / 255.0, 0xdd / 255.0, 0.45 };;
    case Color::LIME:
        return { 0x50 / 255.0, 0xf0 / 255.0, 0x39 / 255.0, 0.45 };;
    default:
        return {0, 0, 0, 0};
    }
}


void hSetPlayerColor(void* something, unsigned char player_id, unsigned char color)
{
    GameData = something;
    players[player_id].color = (Color)color;
    oSetPlayerColor(something, player_id, color);
}

template<typename FnPtr>
FnPtr install_hook(FnPtr original, FnPtr hook)
{
    return (FnPtr)untyped_install_hook((void*)original, (void*)hook);
}

bool fart = true;

ID3D11DeviceContext* context;

ID3D11RenderTargetView* g_mainRenderTargetView;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static WNDPROC OriginalWndProcHandler = nullptr;
HWND window = nullptr;

bool IsKeyPressed(unsigned char key)
{
    return GetAsyncKeyState(key) & 0x8000;
}

LRESULT WINAPI hWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(IsKeyPressed(VK_F1))
	{
        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
        return true;
	}
		
	return CallWindowProc(OriginalWndProcHandler, hWnd, msg, wParam, lParam);
}

bool show_status = false;

long __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    static bool init = false;

    if (!init)
    {
        DXGI_SWAP_CHAIN_DESC desc;
        pSwapChain->GetDesc(&desc);

        ID3D11Device* device;
        pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device);

        device->GetImmediateContext(&context);

        ID3D11Texture2D* pBackBuffer;
        pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        device->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();

        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(desc.OutputWindow);
        window = desc.OutputWindow;
        OriginalWndProcHandler = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)hWndProc);
        ImGui_ImplDX11_Init(device, context);

        init = true;
    }

    if (!IsKeyPressed(VK_F1))
    {
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("People of color");

    for (auto& [id, info] : players)
    {
        ImGui::PushID(id + 1);
        ImGui::PushStyleColor(ImGuiCol_Border, cid2color(info.color));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, cid2color(info.color));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, cid2color(info.color));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, cid2color(info.color));
        ImGui::BeginGroupPanel(cid2text(info.color), {-1, -1});
        ImGui::RadioButton("Trusted", (int*)&info.trust, TRUSTED); ImGui::SameLine();
        ImGui::RadioButton("Plausible", (int*)&info.trust, PLAUSIBLE); ImGui::SameLine();
        ImGui::RadioButton("Neutral", (int*)&info.trust, NEUTRAL); ImGui::SameLine();
        ImGui::RadioButton("Suspicious", (int*)&info.trust, SUSPICIOUS); ImGui::SameLine();
        ImGui::RadioButton("Impostor", (int*)&info.trust, IMPOSTOR);
        if(ImGui::Button("Remove"))
        {
            players.erase(id);
            break;
        } ImGui::SameLine();
        if (ImGui::Button("Revive"))
        {
            oRevive(oGetPlayerInfoById(GameData, id)->fields._object);
            break;
        }
        ImGui::EndGroupPanel();
        ImGui::PopStyleColor(4);
        ImGui::PopID();
    }
	if(ImGui::Button("Clear"))
	{
        players.clear();
	}
    ImGui::Checkbox("Cheat?", &show_status);
	if(GameData && show_status)
	{
        ImGui::Text("Impostors:");
		for(int i=0; i<10; i++)
		{
            GameData_PlayerInfo* info = oGetPlayerInfoById(GameData, i);
			if(!info)
			{
                break;
			}
			if(info->fields.IsImpostor)
			{
                ImGui::Text(cid2text((Color)info->fields.ColorId));
			}
		}
		if(ImGui::Button("kys"))
		{
            PlayerControl* first = oGetPlayerInfoById(GameData, 0)->fields._object;
            oMurderPlayer(first, first);
		}
	}
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();

    context->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);
}

/*void hRemovePlayer(void* GameData, unsigned char id)
{
    const char* col = cid2text(players.at(id).color);
    MessageBoxA(NULL, col, "wew", NULL);
    oGameData_RemovePlayer(GameData, id);
}*/

void amain()
{
    const auto SetPlayerColor = (SetPlayerColorPtr)Pocket::Sigscan::FindPattern("gameassembly.dll", "55 8B EC 53 8B 5D 0C 6A 00 53 FF 75 08 E8 ?? ?? ?? ?? 83 C4 0C 85 C0 74 06 8A 4D 10 88 48 10 0F B6 CB B8");
    oGetPlayerInfoById = (GetPlayerInfoByIdPtr)Pocket::Sigscan::FindPattern("gameassembly.dll", "55 8B EC 80 3D ?? ?? ?? ?? ?? 75 15 FF 35 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 C4 04 C6 05 ?? ?? ?? ?? ?? 8A 55 0C 53 56 57 80 FA FF 74 6B 8B 45 08 33 FF 33 C9 8B 40 24 85 C0 74 64 8B F0 8D 5F 10 90 3B 48 0C");
    oSetPlayerColor = (SetPlayerColorPtr)install_hook(SetPlayerColor, hSetPlayerColor);
    oMurderPlayer = (MurderPlayerPtr)((unsigned int)GetModuleHandleA("gameassembly.dll") + 0x261AC0);
    oRevive = (RevivePtr)((unsigned int)GetModuleHandleA("gameassembly.dll") + 0x262E20);

    //oGameData_RemovePlayer = (GameData_RemovePlayerPtr)install_hook(GameData_RemovePlayer, hRemovePlayer);
    assert(kiero::init(kiero::RenderType::Auto) == kiero::Status::Success);
    assert(kiero::bind(8, (void**)&oPresent, hkPresent11) == kiero::Status::Success);
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DWORD id;
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)amain, NULL, NULL, &id);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
    default:
        break;
    }
    return TRUE;
}

