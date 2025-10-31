#include <globals.hpp>
#include <gui.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <nexus/Nexus.h>
#include <settings.hpp>
#include <string>
#include <vector>
#include <windows.h>

void addon_load(AddonAPI *api_p);
void addon_unload();

BOOL APIENTRY dll_main(const HMODULE hModule, const DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        self_module = hModule;
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }
    return TRUE;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" __declspec(dllexport) AddonDefinition *GetAddonDef()
{
    addon_def.Signature = 3382683172;
    addon_def.APIVersion = NEXUS_API_VERSION;
    addon_def.Name = "App Launcher";
    addon_def.Version.Major = 0;
    addon_def.Version.Minor = 5;
    addon_def.Version.Build = 2;
    addon_def.Version.Revision = 0;
    addon_def.Author = "Seres67";
    addon_def.Description = "An addon that launches other programs when you launch the game.";
    addon_def.Load = addon_load;
    addon_def.Unload = addon_unload;
    addon_def.Flags = EAddonFlags_None;
    addon_def.Provider = EUpdateProvider_GitHub;
    addon_def.UpdateLink = "https://github.com/Seres67/nexus_app_launcher";

    return &addon_def;
}

unsigned int wnd_proc(HWND__ *hWnd, const unsigned int uMsg, [[maybe_unused]] WPARAM wParam,
                      [[maybe_unused]] LPARAM lParam)
{
    if (!game_handle)
        game_handle = hWnd;
    if (uMsg == WM_CLOSE || uMsg == WM_DESTROY || uMsg == WM_QUIT) {
        if (game_handle != nullptr) {
            if (Settings::kill_processes_on_close) {
                api->Log(ELogLevel_INFO, addon_name, "killing every program on exit...");
                for (auto &[pi, si] : processes) {
                    char log[256];
                    sprintf_s(log, "killing process %d", pi.dwProcessId);
                    api->Log(ELogLevel_DEBUG, addon_name, log);
                    TerminateProcess(pi.hProcess, 0);
                    WaitForSingleObject(pi.hProcess, INFINITE);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            api->Log(ELogLevel_INFO, addon_name, "starting every program on exit...");
            for (auto &[path, working, arguments] : Settings::exit_programs_path) {
                char log[256];
                sprintf_s(log, "trying to start program at %s", path.c_str());
                api->Log(ELogLevel_DEBUG, addon_name, log);
                PROCESS_INFORMATION pi;
                STARTUPINFOA si;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));
                std::string cmd(" " + arguments);
                CreateProcessA(path.c_str(), const_cast<char *>(cmd.c_str()), nullptr, nullptr, false, DETACHED_PROCESS,
                               nullptr, working.c_str(), &si, &pi);
            }
            api->Log(ELogLevel_INFO, addon_name, "launched every program on exit!");
        }
        api->Log(ELogLevel_DEBUG, addon_name, "after handle check");
    }
    return uMsg;
}

void addon_load(AddonAPI *api_p)
{
    api = api_p;

    ImGui::SetCurrentContext(static_cast<ImGuiContext *>(api->ImguiContext));
    ImGui::SetAllocatorFunctions(reinterpret_cast<void *(*)(size_t, void *)>(api->ImguiMalloc),
                                 reinterpret_cast<void (*)(void *, void *)>(api->ImguiFree)); // on imgui 1.80+
    api->Renderer.Register(ERenderType_Render, addon_render);
    api->Renderer.Register(ERenderType_OptionsRender, addon_options);
    api->WndProc.Register(wnd_proc);

    Settings::settings_path = api->Paths.GetAddonDirectory("app_launcher/settings.json");
    if (std::filesystem::exists(Settings::settings_path)) {
        Settings::load(Settings::settings_path);
    } else {
        Settings::json_settings[Settings::KILL_PROCESSES_ON_CLOSE] = Settings::kill_processes_on_close;
        Settings::json_settings[Settings::START_PROGRAMS_PATH] = Settings::start_programs_path;
        Settings::json_settings[Settings::EXIT_PROGRAMS_PATH] = Settings::exit_programs_path;
        Settings::save(Settings::settings_path);
    }
    GetEnvironmentVariableA("Path", path, 4096);
    api->Log(ELogLevel_INFO, addon_name, "addon loaded!");
}

void addon_unload()
{
    api->Log(ELogLevel_INFO, addon_name, "unloading addon...");
    api->Renderer.Deregister(addon_render);
    api->Renderer.Deregister(addon_options);
    api->WndProc.Deregister(wnd_proc);
    Settings::start_programs_path.clear();
    Settings::exit_programs_path.clear();
    api = nullptr;
}
