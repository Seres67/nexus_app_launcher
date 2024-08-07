#include <globals.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
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

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD ul_reason_for_call, LPVOID lpReserved)
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
    AddonDef.Version.Minor = 3;
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

void AddonLoad(AddonAPI *api)
{
    API = api;

    API->WndProc.Register(WndProc);

    ImGui::SetCurrentContext((ImGuiContext *)API->ImguiContext);
    ImGui::SetAllocatorFunctions((void *(*)(size_t, void *))API->ImguiMalloc,
                                 (void (*)(void *, void *))API->ImguiFree); // on imgui 1.80+
    API->Renderer.Register(ERenderType_Render, AddonRender);
    API->Renderer.Register(ERenderType_OptionsRender, AddonOptions);

    Settings::SettingsPath = API->Paths.GetAddonDirectory("app_launcher/settings.json");
    if (std::filesystem::exists(Settings::SettingsPath)) {
        Settings::Load(Settings::SettingsPath);
    } else {
        Settings::json_settings[Settings::IS_ADDON_ENABLED] = Settings::IsAddonEnabled;
        Settings::json_settings[Settings::KILL_PROCESSES_ON_CLOSE] = Settings::KillProcessesOnClose;
        Settings::json_settings[Settings::PROGRAMS_PATH] = Settings::programsPath;
        Settings::Save(Settings::SettingsPath);
    }
    API->Log(ELogLevel_INFO, "App Launcher", "addon loaded!");
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
        for (auto &[path, arguments] : Settings::programsPath) {
            processes.emplace_back();
            ZeroMemory(&processes.back().si, sizeof(processes.back().si));
            processes.back().si.cb = sizeof(processes.back().si);
            ZeroMemory(&processes.back().pi, sizeof(processes.back().pi));
            if (arguments.empty()) {
                CreateProcessA(path.c_str(), nullptr, nullptr, nullptr, false, 0, nullptr, nullptr,
                               &processes.back().si, &processes.back().pi);
            } else {
                std::string cmd(" " + arguments);
                CreateProcessA(path.c_str(), const_cast<char *>(cmd.c_str()), nullptr, nullptr, false, 0, nullptr,
                               nullptr, &processes.back().si, &processes.back().pi);
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

void killProcess(const int i)
{
    TerminateProcess(processes[i].pi.hProcess, 0);
    WaitForSingleObject(processes[i].pi.hProcess, INFINITE);
    CloseHandle(processes[i].pi.hProcess);
    CloseHandle(processes[i].pi.hThread);
    processes.erase(processes.begin() + i);
}

template <typename Out> void split(const std::string &s, const char delim, Out result)
{
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}

std::vector<std::string> split(const std::string &s, const char delim)
{
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

std::string get_path()
{
    char path[10240];
    GetEnvironmentVariableA("Path", path, 10240);
    return path;
}

std::string get_program_path(const std::string &program, const std::string &path)
{
    for (const auto paths = split(path, ';'); auto p : paths) {
        p.append("\\").append(program);
        char log[256];
        sprintf_s(log, "Checking if program exists at %s", p.c_str());
        API->Log(ELogLevel_DEBUG, "App Launcher", log);
        if (std::filesystem::exists(p)) {
            return p;
        }
    }
    return program;
}

char newProgram[256] = {0};
char newArguments[256] = {0};
char editProgramPath[256] = {0};
char editProgramArguments[256] = {0};
bool isProgramValid = true;
ImGui::FileBrowser fileBrowser;
int editProgram = -1;
void AddonOptions()
{
    if (ImGui::Checkbox("Enabled##Widget", &Settings::IsAddonEnabled)) {
        Settings::json_settings[Settings::IS_ADDON_ENABLED] = Settings::IsAddonEnabled;
        Settings::Save(Settings::SettingsPath);
    }
    if (ImGui::Checkbox("Kill Processes On Close##Widget", &Settings::KillProcessesOnClose)) {
        Settings::json_settings[Settings::KILL_PROCESSES_ON_CLOSE] = Settings::KillProcessesOnClose;
        Settings::Save(Settings::SettingsPath);
    }
    if (ImGui::CollapsingHeader("Programs Path##ProgramsPathHeader")) {
        for (auto i = 0; i < Settings::programsPath.size(); i++) {
            ImGui::PushID(i);
            if (editProgram == i) {
                ImGui::InputText("Program Path##ProgramPathInput", newProgram, 256);
                ImGui::InputText("Program Arguments##ProgramArgumentsInput", newArguments, 256);
                if (ImGui::Button("Confirm##ConfirmButton")) {
                    Settings::programsPath[i].path = newProgram;
                    Settings::programsPath[i].arguments = newArguments;
                    Settings::json_settings[Settings::PROGRAMS_PATH] = Settings::programsPath;
                    Settings::Save(Settings::SettingsPath);
                    memset(newProgram, 0, 256);
                    memset(newArguments, 0, 256);
                    isProgramValid = true;
                    editProgram = -1;
                    if (Settings::KillProcessesOnClose)
                        killProcess(i);
                }
            } else {
                if (std::filesystem::exists(Settings::programsPath[i].path)) {
                    ImGui::Text(Settings::programsPath[i].path.c_str());
                    ImGui::SameLine();
                    ImGui::Text(Settings::programsPath[i].arguments.c_str());
                } else {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), Settings::programsPath[i].path.c_str());
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Path does not exist.");
                    }
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), Settings::programsPath[i].arguments.c_str());
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Edit")) {
                editProgram = i;
                strcpy_s(editProgramPath, Settings::programsPath[i].path.c_str());
                strcpy_s(editProgramArguments, Settings::programsPath[i].arguments.c_str());
            }
            ImGui::SameLine();
            if (ImGui::Button("X")) {
                if (Settings::KillProcessesOnClose && processes.size() > i)
                    killProcess(i);
                Settings::remove_program(i);
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
                if (const auto pos = program.find_first_of('"'); pos != std::string::npos) {
                    program.erase(pos, 1);
                    program.erase(program.find_last_of('"'), 1);
                }
                if (program.find("/\\") == std::string::npos) {
                    API->Log(ELogLevel_DEBUG, "App Launcher", "Trying to find program in PATH...");
                    program = get_program_path(program, get_path());
                }
                Settings::add_program(program, arguments);
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