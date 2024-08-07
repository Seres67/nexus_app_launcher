#include <globals.hpp>
#include <imgui/imgui.h>
#include <imgui-filebrowser/imfilebrowser.h>
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

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
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
    AddonDef.Version.Minor = 2;
    AddonDef.Version.Build = 0;
    AddonDef.Version.Revision = 0;
    AddonDef.Author = "Seres67";
    AddonDef.Description = "An addon that launches other programs when you launch the game.";
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
    ImGui::SetAllocatorFunctions((void *(*)(size_t, void *))API->ImguiMalloc,
                                 (void (*)(void *, void *))API->ImguiFree); // on imgui 1.80+
    API->Renderer.Register(ERenderType_Render, AddonRender);
    API->Renderer.Register(ERenderType_OptionsRender, AddonOptions);

    SettingsPath = API->Paths.GetAddonDirectory("app_launcher/settings.json");
    if (std::filesystem::exists(SettingsPath)) {
        Settings::Load(SettingsPath);
    } else {
        Settings::json_settings[Settings::IS_ADDON_ENABLED] = Settings::IsAddonEnabled;
        Settings::json_settings[Settings::KILL_PROCESSES_ON_CLOSE] = Settings::KillProcessesOnClose;
        Settings::json_settings[Settings::PROGRAMS_PATH] = Settings::programsPath;
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
    API->Renderer.Deregister(AddonOptions);
}

bool opened = false;
void AddonRender()
{
    if (handle != nullptr && !opened) {
        for (auto &program : Settings::programsPath) {
            processes.emplace_back();
            ZeroMemory(&processes.back().si, sizeof(processes.back().si));
            processes.back().si.cb = sizeof(processes.back().si);
            ZeroMemory(&processes.back().pi, sizeof(processes.back().pi));
            if (program.arguments.empty()) {
                CreateProcessA(program.path.c_str(), nullptr, nullptr, nullptr, false, 0, nullptr, nullptr,
                               &processes.back().si, &processes.back().pi);
            } else {
                std::string cmd(" " + program.arguments);
                CreateProcessA(program.path.c_str(), const_cast<char *>(cmd.c_str()), nullptr, nullptr, false, 0,
                               nullptr, nullptr, &processes.back().si, &processes.back().pi);
            }
        }
        opened = true;
        std::thread(
            []()
            {
                API->Renderer.Deregister(AddonRender);
                API->WndProc.Deregister(WndProc);
                API->Log(ELogLevel_INFO, "App Launcher", "launched every program & deregistered renderer & wndproc!");
            })
            .detach();
    }
}

char newProgram[256] = {0};
char newArguments[256] = {0};
bool isProgramValid = true;
ImGui::FileBrowser fileBrowser;
void AddonOptions()
{
    if (ImGui::Checkbox("Enabled##Widget", &Settings::IsAddonEnabled)) {
        Settings::json_settings[Settings::IS_ADDON_ENABLED] = Settings::IsAddonEnabled;
        Settings::Save(SettingsPath);
    }
    if (ImGui::Checkbox("Kill Processes On Close##Widget", &Settings::KillProcessesOnClose)) {
        Settings::json_settings[Settings::KILL_PROCESSES_ON_CLOSE] = Settings::KillProcessesOnClose;
        Settings::Save(SettingsPath);
    }
    if (ImGui::CollapsingHeader("Programs Path##ProgramsPathHeader")) {
        for (auto i = 0; i < Settings::programsPath.size(); i++) {
            ImGui::PushID(i);
            ImGui::Text(Settings::programsPath[i].path.c_str());
            ImGui::SameLine();
            ImGui::Text(Settings::programsPath[i].arguments.c_str());
            ImGui::SameLine();
            if (ImGui::Button("X")) {
                if (Settings::KillProcessesOnClose && processes.size() > i) {
                    TerminateProcess(processes[i].pi.hProcess, 0);
                    WaitForSingleObject(processes[i].pi.hProcess, INFINITE);
                    CloseHandle(processes[i].pi.hProcess);
                    CloseHandle(processes[i].pi.hThread);
                    processes.erase(processes.begin() + i);
                }
                Settings::programsPath.erase(Settings::programsPath.begin() + i);
                Settings::json_settings[Settings::PROGRAMS_PATH] = Settings::programsPath;
                Settings::Save(SettingsPath);
                continue;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Remove program from list and kills it if kill processes on close is enabled.");
            }
            ImGui::PopID();
        }
    }
    if (ImGui::CollapsingHeader("Add program##AddProgramHeader")) {
        if (ImGui::Button("Open File Picker##OpenFilePickerButton")) {
            fileBrowser.SetTypeFilters({".exe"});
            fileBrowser.Open();
        }
        fileBrowser.Display();
        if (fileBrowser.HasSelected()) {
            strcpy_s(newProgram, (char *)fileBrowser.GetSelected().u8string().c_str());
        }
        ImGui::NewLine();
        ImGui::InputText("Program Path##ProgramPathInput", newProgram, 256);
        ImGui::InputText("Program Arguments##ProgramArgumentsInput", newArguments, 256);
        if (ImGui::Button("Add program##AddProgramButton")) {
            std::string program(newProgram);
            std::string arguments(newArguments);
            if (program.find(".exe") == std::string::npos) {
                isProgramValid = false;
            } else {
                auto pos = program.find_first_of('"');
                if (pos != std::string::npos) {
                    program.erase(pos, 1);
                    program.erase(program.find_last_of('"'), 1);
                }
                Settings::programsPath.emplace_back(program, arguments);
                Settings::json_settings[Settings::PROGRAMS_PATH] = Settings::programsPath;
                Settings::Save(SettingsPath);
                fileBrowser.ClearSelected();
                memset(newProgram, 0, 256);
                memset(newArguments, 0, 256);
                isProgramValid = true;
            }
        }
        if (!isProgramValid) {
            ImGui::SameLine();
            ImGui::Text("Path must lead to an executable file. (.exe)");
        }
    }
}