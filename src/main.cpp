#include <nexus/Nexus.h>
#include <windows.h>

HMODULE hSelf;

void AddonLoad(AddonAPI *aApi);
void AddonUnload();
void AddonRender();

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        hSelf = hModule;
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }
    return TRUE;
}

AddonDefinition AddonDef{};
extern "C" __declspec(dllexport) AddonDefinition *GetAddonDef()
{
    AddonDef.Signature = -912284124;
    AddonDef.APIVersion = NEXUS_API_VERSION;
    AddonDef.Name = "App Launcher";
    AddonDef.Version.Major = 0;
    AddonDef.Version.Minor = 1;
    AddonDef.Version.Build = 0;
    AddonDef.Version.Revision = 0;
    AddonDef.Author = "Seres67";
    AddonDef.Description =
        "An addon that launches other programs when you launch the game.";
    AddonDef.Load = AddonLoad;
    AddonDef.Unload = AddonUnload;
    AddonDef.Flags = EAddonFlags_None;
    AddonDef.Provider = EUpdateProvider_GitHub;
    AddonDef.UpdateLink = "https://github.com/Seres67/nexus_app_launcher";

    return &AddonDef;
}

HWND handle = nullptr;
unsigned int WndProc(HWND hWnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!handle)
        handle = hWnd;
    return uMsg;
}

void AddonLoad(AddonAPI *api)
{
    api->WndProc.Register(WndProc);
    api->Renderer.Register(ERenderType_Render, AddonRender);
}

void AddonUnload() {}

bool opened = false;
void AddonRender()
{
    if (handle != nullptr && !opened) {
        ShellExecuteA(handle, "open", "../Noita.v06.04.2024/noita.exe", nullptr,
                      nullptr, 0);
        opened = true;
    }
}
