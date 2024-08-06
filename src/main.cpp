#include <globals.hpp>
#include <imgui/imgui.h>
#include <nexus/Nexus.h>
#include <settings.hpp>
#include <string>
#include <vector>
#include <windows.h>

HMODULE hSelf;

void AddonLoad(AddonAPI *aApi);
void AddonUnload();
void AddonRender();
void AddonOptions();

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

std::filesystem::path SettingsPath;
void AddonLoad(AddonAPI *api)
{
    API = api;

    API->WndProc.Register(WndProc);

    ImGui::SetCurrentContext((ImGuiContext *)API->ImguiContext);
    ImGui::SetAllocatorFunctions(
        (void *(*)(size_t, void *))API->ImguiMalloc,
        (void (*)(void *, void *))API->ImguiFree); // on imgui 1.80+
    API->Renderer.Register(ERenderType_Render, AddonRender);
    API->Renderer.Register(ERenderType_OptionsRender, AddonOptions);

    SettingsPath = API->Paths.GetAddonDirectory("app_launcher/settings.json");
    char log[256];
    sprintf_s(log, "settings path: %s", SettingsPath.string().c_str());
    API->Log(ELogLevel_INFO, "App Launcher", log);
    if (std::filesystem::exists(SettingsPath)) {
        Settings::Load(SettingsPath);
    } else {
        Settings::json_settings[Settings::IS_ADDON_ENABLED] =
            Settings::IsAddonEnabled;
        Settings::Save(SettingsPath);
    }
    API->Log(ELogLevel_INFO, "App Launcher", "loaded!");
}

typedef struct
{
    PROCESS_INFORMATION pi;
    STARTUPINFOA si;
} process;
std::vector<process> processes;
void AddonUnload()
{
    if (Settings::KillProcessesOnClose) {
        for (auto &[pi, si] : processes) {
            TerminateProcess(pi.hProcess, 0);
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }
}

bool opened = false;
void AddonRender()
{
    if (handle != nullptr && !opened) {
        for (auto &program : Settings::programsPath) {
            char log[256];
            sprintf_s(log, "starting program: %s", program.c_str());
            API->Log(ELogLevel_INFO, "App Launcher", log);

            processes.emplace_back();
            ZeroMemory(&processes.back().si, sizeof(processes.back().si));
            processes.back().si.cb = sizeof(processes.back().si);
            ZeroMemory(&processes.back().pi, sizeof(processes.back().pi));
            CreateProcessA(program.c_str(), nullptr, nullptr, nullptr, false, 0,
                           nullptr, nullptr, &processes.back().si,
                           &processes.back().pi);
        }
        opened = true;
        std::thread(
            []()
            {
                API->Renderer.Deregister(AddonRender);
                API->WndProc.Deregister(WndProc);
                API->Log(ELogLevel_INFO, "App Launcher",
                         "deregistered renderer & wndproc!");
            })
            .detach();
    }
}

char newProgram[256] = {0};
void AddonOptions()
{
    ImGui::TextDisabled("App Launcher");
    if (ImGui::Checkbox("Enabled##Widget", &Settings::IsAddonEnabled)) {
        Settings::json_settings[Settings::IS_ADDON_ENABLED] =
            Settings::IsAddonEnabled;
        Settings::Save(SettingsPath);
    }
    if (ImGui::Checkbox("Kill Processes On Close##Widget",
                        &Settings::KillProcessesOnClose)) {
        Settings::json_settings[Settings::KILL_PROCESSES_ON_CLOSE] =
            Settings::KillProcessesOnClose;
        Settings::Save(SettingsPath);
    }
    if (ImGui::CollapsingHeader("Programs Path")) {
        ImGui::Text("Programs Path");
        for (auto &program : Settings::programsPath) {
            ImGui::Text(program.c_str());
        }
        ImGui::InputText("Add program##ProgramInput", newProgram, 256);
        if (ImGui::Button("Add program")) {
            Settings::programsPath.emplace_back(newProgram);
            Settings::json_settings[Settings::PROGRAMS_PATH] =
                Settings::programsPath;
            Settings::Save(SettingsPath);
            memset(newProgram, 0, 256);
        }
    }
}