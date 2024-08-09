#include <globals.hpp>
#include <gui.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <nexus/Nexus.h>
#include <settings.hpp>
#include <string>
#include <vector>
#include <windows.h>

void addon_load(AddonAPI *api);
void addon_unload();
void addon_render();
void addon_options();

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD ul_reason_for_call, LPVOID lpReserved)
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

extern "C" __declspec(dllexport) AddonDefinition *GetAddonDef()
{
    addon_def.Signature = -912284124;
    addon_def.APIVersion = NEXUS_API_VERSION;
    addon_def.Name = "App Launcher";
    addon_def.Version.Major = 0;
    addon_def.Version.Minor = 4;
    addon_def.Version.Build = 0;
    addon_def.Version.Revision = 0;
    addon_def.Author = "Seres67";
    addon_def.Description = "An addon that launches other programs when you launch the game.";
    addon_def.Load = addon_load;
    addon_def.Unload = addon_unload;
    addon_def.Flags = (EAddonFlags)8;
    addon_def.Provider = EUpdateProvider_GitHub;
    addon_def.UpdateLink = "https://github.com/Seres67/nexus_app_launcher";

    return &addon_def;
}

void create_process(const std::string &path, const std::string &arguments)
{
    processes.emplace_back();
    ZeroMemory(&processes.back().si, sizeof(processes.back().si));
    processes.back().si.cb = sizeof(processes.back().si);
    ZeroMemory(&processes.back().pi, sizeof(processes.back().pi));
    std::string cmd(" " + arguments);
    CreateProcessA(path.c_str(), const_cast<char *>(cmd.c_str()), nullptr, nullptr, false, 0, nullptr, nullptr,
                   &processes.back().si, &processes.back().pi);
}

unsigned int wnd_proc(HWND__ *hWnd, const unsigned int uMsg, [[maybe_unused]] WPARAM wParam,
                      [[maybe_unused]] LPARAM lParam)
{
    if (!game_handle)
        game_handle = hWnd;
    if (uMsg == WM_CLOSE || uMsg == WM_DESTROY || uMsg == WM_QUIT) {
        if (game_handle != nullptr) {
            if (Settings::kill_processes_on_close) {
                API->Log(ELogLevel_INFO, "App Launcher", "killing every program on exit...");
                for (auto &[pi, si] : processes) {
                    char log[256];
                    sprintf_s(log, "killing process %d", pi.dwProcessId);
                    API->Log(ELogLevel_DEBUG, "App Launcher", log);
                    TerminateProcess(pi.hProcess, 0);
                    WaitForSingleObject(pi.hProcess, INFINITE);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            API->Log(ELogLevel_INFO, "App Launcher", "starting every program on exit...");
            for (auto &[path, arguments] : Settings::exit_programs_path) {
                char log[256];
                sprintf_s(log, "trying to start program at %s", path.c_str());
                API->Log(ELogLevel_DEBUG, "App Launcher", log);
                PROCESS_INFORMATION pi;
                STARTUPINFOA si;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));
                std::string cmd(" " + arguments);
                CreateProcessA(path.c_str(), const_cast<char *>(cmd.c_str()), nullptr, nullptr, false, 0, nullptr,
                               nullptr, &si, &pi);
            }
            std::thread(
                []()
                {
                    API->WndProc.Deregister(wnd_proc);
                    API->Log(ELogLevel_INFO, "App Launcher", "launched every program on exit & deregistered wndproc!");
                })
                .detach();
        } else {
            API->Log(ELogLevel_DEBUG, "App Launcher", "handle is null");
        }
    }
    return uMsg;
}

void addon_load(AddonAPI *api)
{
    API = api;

    API->WndProc.Register(wnd_proc);

    ImGui::SetCurrentContext(static_cast<ImGuiContext *>(API->ImguiContext));
    ImGui::SetAllocatorFunctions(static_cast<void *(*)(size_t, void *)>(API->ImguiMalloc),
                                 static_cast<void (*)(void *, void *)>(API->ImguiFree)); // on imgui 1.80+
    API->Renderer.Register(ERenderType_Render, addon_render);
    API->Renderer.Register(ERenderType_OptionsRender, addon_options);

    Settings::settings_path = API->Paths.GetAddonDirectory("app_launcher/settings.json");
    if (std::filesystem::exists(Settings::settings_path)) {
        Settings::load(Settings::settings_path);
    } else {
        Settings::json_settings[Settings::IS_ADDON_ENABLED] = Settings::is_addon_enabled;
        Settings::json_settings[Settings::KILL_PROCESSES_ON_CLOSE] = Settings::kill_processes_on_close;
        Settings::json_settings[Settings::START_PROGRAMS_PATH] = Settings::start_programs_path;
        Settings::json_settings[Settings::EXIT_PROGRAMS_PATH] = Settings::exit_programs_path;
        Settings::save(Settings::settings_path);
    }
    GetEnvironmentVariableA("Path", path, 10240);
    API->Log(ELogLevel_INFO, "App Launcher", "addon loaded!");
}

void addon_unload()
{
    API->Log(ELogLevel_INFO, "App Launcher", "unloading addon...");
    API->Renderer.Deregister(addon_options);
    API = nullptr;
}

void addon_render()
{
    if (game_handle != nullptr && !started_programs) {
        for (auto &[path, arguments] : Settings::start_programs_path)
            create_process(path, arguments);
        started_programs = true;
        std::thread(
            []()
            {
                API->Renderer.Deregister(addon_render);
                API->Log(ELogLevel_INFO, "App Launcher", "launched every program & deregistered renderer!");
            })
            .detach();
    }
}

void addon_options()
{
    display_active_option();
    display_kill_processes_on_close_option();
    if (ImGui::CollapsingHeader("Programs Path##ProgramsPathHeader")) {
        display_start_programs_option();
        ImGui::NewLine();
        display_exit_programs_option();
    }
    if (ImGui::CollapsingHeader("Add program##AddProgramHeader")) {
        display_add_program_option();
    }
}
