#include "nexus/Nexus.h"
#include <cstring>
#include <globals.hpp>
#include <gui.hpp>
#include <imgui/imgui.h>
#include <settings.hpp>
#include <tchar.h>
#include <thread>

void display_kill_processes_on_close_option()
{
    if (ImGui::Checkbox("Kill Processes On Close##Widget", &Settings::kill_processes_on_close)) {
        Settings::json_settings[Settings::KILL_PROCESSES_ON_CLOSE] = Settings::kill_processes_on_close;
        Settings::save(Settings::settings_path);
    }
}

void display_start_programs_option()
{
    for (auto i = 0; i < Settings::start_programs_path.size(); i++) {
        ImGui::PushID(i);
        if (edit_program == i) {
            ImGui::Checkbox("Start on game exit##AppLauncherStartProgramOnExit", &edit_start_on_exit);
            ImGui::InputText("Program Path##AppLauncherProgramPathInput", edit_program_path, 256);
            ImGui::InputText("Program Working Directory##AppLauncherProgramWorkingDirectoryInput",
                             edit_program_working_dir, 256);
            ImGui::InputText("Program Arguments##AppLauncherProgramArgumentsInput", edit_program_arguments, 256);
            if (ImGui::Button("Confirm##AppLauncherConfirmButton")) {
                if (edit_start_on_exit) {
                    program_to_swap = i;
                }
                Settings::start_programs_path[i].path = edit_program_path;
                Settings::start_programs_path[i].working_dir = edit_program_working_dir;
                Settings::start_programs_path[i].arguments = edit_program_arguments;
                Settings::json_settings[Settings::START_PROGRAMS_PATH] = Settings::start_programs_path;
                Settings::save(Settings::settings_path);
                memset(edit_program_path, 0, 256);
                memset(edit_program_working_dir, 0, 256);
                memset(edit_program_arguments, 0, 256);
                is_program_valid = true;
                edit_program = -1;
                if (Settings::kill_processes_on_close)
                    kill_process(i);
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##AppLauncherCancelButton")) {
                memset(edit_program_path, 0, 256);
                memset(edit_program_working_dir, 0, 256);
                memset(edit_program_arguments, 0, 256);
                is_program_valid = true;
                edit_program = -1;
            }
        } else {
            if (std::filesystem::exists(Settings::start_programs_path[i].path)) {
                ImGui::Text("%s", Settings::start_programs_path[i].path.c_str());
                ImGui::Text("working dir: %s", Settings::start_programs_path[i].working_dir.c_str());
                ImGui::Text("arguments: %s", Settings::start_programs_path[i].arguments.c_str());
            } else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", Settings::start_programs_path[i].path.c_str());
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Path does not exist.");
                }
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "working dir: %s",
                                   Settings::start_programs_path[i].working_dir.c_str());
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "arguments: %s",
                                   Settings::start_programs_path[i].arguments.c_str());
            }
        }
        if (edit_program == -1 && ImGui::Button("Edit##AppLauncherEditStartProgram")) {
            edit_program = i;
            strcpy_s(edit_program_path, Settings::start_programs_path[i].path.c_str());
            strcpy_s(edit_program_working_dir, Settings::start_programs_path[i].working_dir.c_str());
            strcpy_s(edit_program_arguments, Settings::start_programs_path[i].arguments.c_str());
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete##AppLauncherDeleteStartProgram")) {
            if (Settings::kill_processes_on_close && processes.size() > i)
                kill_process(i);
            Settings::remove_start_program(i);
            edit_program = -1;
            continue;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Remove program from list and kills it if kill processes on close is enabled.");
        }
        ImGui::NewLine();
        ImGui::PopID();
    }
    if (program_to_swap != -1) {
        std::string log("Swapping " + std::to_string(program_to_swap));
        api->Log(ELogLevel_INFO, addon_name, log.c_str());
        std::string path(Settings::start_programs_path[program_to_swap].path);
        std::string working_dir(Settings::start_programs_path[program_to_swap].working_dir);
        std::string arguments(Settings::start_programs_path[program_to_swap].arguments);
        kill_process(program_to_swap);
        Settings::remove_start_program(program_to_swap);
        Settings::add_exit_program(path, working_dir, arguments);
        program_to_swap = -1;
        edit_start_on_exit = false;
    }
}

void display_exit_programs_option()
{
    for (auto i = 0; i < Settings::exit_programs_path.size(); i++) {
        ImGui::PushID(("exit" + std::to_string(i)).c_str());
        if (edit_exit_program == i) {
            ImGui::Checkbox("Start on game exit##AppLauncherStartProgramOnExit", &edit_exit_start_on_exit);
            ImGui::InputText("Program Path##AppLauncherProgramPathInput", edit_program_path, 256);
            ImGui::InputText("Program Working Directory##AppLauncherProgramWorkingDirectoryInput",
                             edit_program_working_dir, 256);
            ImGui::InputText("Program Arguments##AppLauncherProgramArgumentsInput", edit_program_arguments, 256);
            if (ImGui::Button("Confirm##AppLauncherConfirmButton")) {
                if (!edit_exit_start_on_exit) {
                    program_to_swap = i;
                }
                Settings::exit_programs_path[i].path = edit_program_path;
                Settings::exit_programs_path[i].working_dir = edit_program_working_dir;
                Settings::exit_programs_path[i].arguments = edit_program_arguments;
                Settings::json_settings[Settings::EXIT_PROGRAMS_PATH] = Settings::exit_programs_path;
                Settings::save(Settings::settings_path);
                memset(edit_program_path, 0, 256);
                memset(edit_program_working_dir, 0, 256);
                memset(edit_program_arguments, 0, 256);
                is_program_valid = true;
                edit_exit_program = -1;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##AppLauncherCancelButton")) {
                memset(edit_program_path, 0, 256);
                memset(edit_program_working_dir, 0, 256);
                memset(edit_program_arguments, 0, 256);
                is_program_valid = true;
                edit_exit_program = -1;
            }
        } else {
            if (std::filesystem::exists(Settings::exit_programs_path[i].path)) {
                ImGui::Text("%s", Settings::exit_programs_path[i].path.c_str());
                ImGui::Text("working dir: %s", Settings::exit_programs_path[i].working_dir.c_str());
                ImGui::Text("arguments: %s", Settings::exit_programs_path[i].arguments.c_str());
            } else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", Settings::exit_programs_path[i].path.c_str());
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Path does not exist.");
                }
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "working dir: %s",
                                   Settings::exit_programs_path[i].working_dir.c_str());
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "arguments: %s",
                                   Settings::exit_programs_path[i].arguments.c_str());
            }
        }
        if (edit_exit_program == -1 && ImGui::Button("Edit##AppLauncherEditExitProgram")) {
            edit_exit_program = i;
            strcpy_s(edit_program_path, Settings::exit_programs_path[i].path.c_str());
            strcpy_s(edit_program_working_dir, Settings::exit_programs_path[i].working_dir.c_str());
            strcpy_s(edit_program_arguments, Settings::exit_programs_path[i].arguments.c_str());
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete##AppLauncherDeleteExitProgram")) {
            Settings::remove_exit_program(i);
            edit_exit_program = -1;
            continue;
        }
        ImGui::NewLine();
        ImGui::PopID();
    }
    if (program_to_swap != -1) {
        std::string path(Settings::exit_programs_path[program_to_swap].path);
        std::string working_dir(Settings::exit_programs_path[program_to_swap].working_dir);
        std::string arguments(Settings::exit_programs_path[program_to_swap].arguments);
        Settings::remove_exit_program(program_to_swap);
        Settings::add_start_program(path, working_dir, arguments);
        program_to_swap = -1;
        edit_exit_start_on_exit = false;
    }
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

std::string get_program_path(const std::string &program)
{
    for (const auto paths = split(path, ';'); auto p : paths) {
        p.append("\\").append(program);
        char log[256];
        sprintf_s(log, "Checking if program exists at %s", p.c_str());
        api->Log(ELogLevel_DEBUG, addon_name, log);
        if (std::filesystem::exists(p)) {
            return p;
        }
    }
    return program;
}

std::string get_file_path()
{
    OPENFILENAME ofn{};
    TCHAR file_size[MAX_PATH]{};
    TCHAR initial_directory[MAX_PATH]{};

    _tcscpy_s(initial_directory, Settings::settings_path.string().c_str());

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = static_cast<HWND>(nullptr);
    ofn.lpstrFile = file_size;
    ofn.nMaxFile = sizeof(file_size);
    ofn.lpstrFilter = "All Files (*.*)\0*.*\0"
                      "Executable File (*.exe, *.bat, *.cmd)\0"
                      "*.exe;*.bat;*.cmd\0";
    ofn.nFilterIndex = 2;
    ofn.lpstrInitialDir = initial_directory;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&ofn) == TRUE) {
        if (!std::filesystem::exists(ofn.lpstrFile))
            return {};
        return ofn.lpstrFile;
    }
    return {};
}

void display_add_program_option()
{
    if (ImGui::Button("Open File Picker##AppLauncherOpenFilePickerButton")) {
        std::string file_path = get_file_path();
        if (!file_path.empty()) {
            strcpy_s(new_program, file_path.c_str());
            auto last_slash = file_path.find_first_of('/');
            if (last_slash == std::string::npos)
                last_slash = file_path.find_last_of('\\');
            const std::string &working_directory = file_path.substr(0, last_slash);
            strcpy_s(new_program_working_directory, working_directory.c_str());
        }
    }
    ImGui::NewLine();
    ImGui::Checkbox("Start program on game close##AppLauncherStartOnExit", &start_on_exit);
    ImGui::InputText("Program Path##AppLauncherProgramPathInput", new_program, 256);
    ImGui::InputText("Program Working Directory##AppLauncherProgramWorkingDirectory", new_program_working_directory,
                     256);
    ImGui::InputText("Program Arguments##AppLauncherProgramArgumentsInput", new_arguments, 256);
    if (ImGui::Button("Add program##AppLauncherAddProgramButton")) {
        std::string program(new_program);
        if (program.empty())
            return;
        std::string extension(program.substr(program.find_last_of('.')));
        if (std::ranges::find(supported_extensions.begin(), supported_extensions.end(), extension) ==
            supported_extensions.end()) {
            is_program_valid = false;
        } else {
            if (const auto pos = program.find_first_of('"'); pos != std::string::npos) {
                program.erase(pos, 1);
                program.erase(program.find_last_of('"'), 1);
            }
            if (program.find("/\\") == std::string::npos) {
                api->Log(ELogLevel_DEBUG, addon_name, "Trying to find program in PATH...");
                program = get_program_path(program);
            }
            if (start_on_exit) {
                Settings::add_exit_program(program, new_program_working_directory, new_arguments);
            } else {
                Settings::add_start_program(program, new_program_working_directory, new_arguments);
            }
            memset(new_program, 0, 256);
            memset(new_program_working_directory, 0, 256);
            memset(new_arguments, 0, 256);
            is_program_valid = true;
        }
    }
    if (!is_program_valid) {
        ImGui::SameLine();
        ImGui::Text("Path must lead to an executable file. (.exe)");
    }
}

void create_process(const std::string &path, const std::string &working_dir, const std::string &arguments)
{
    processes.emplace_back();
    ZeroMemory(&processes.back().si, sizeof(processes.back().si));
    processes.back().si.cb = sizeof(processes.back().si);
    ZeroMemory(&processes.back().pi, sizeof(processes.back().pi));
    const std::string cmd(" " + arguments);
    auto last_slash = path.find_first_of('/');
    if (last_slash == std::string::npos)
        last_slash = path.find_last_of('\\');
    const std::string &working_directory = path.substr(0, last_slash);
    api->Log(ELogLevel_DEBUG, addon_name, working_directory.c_str());
    CreateProcessA(path.c_str(), const_cast<char *>(cmd.c_str()), nullptr, nullptr, false, DETACHED_PROCESS, nullptr,
                   working_dir.c_str(), &processes.back().si, &processes.back().pi);
}

void addon_render()
{
    if (game_handle != nullptr && !started_programs) {
        for (auto &[path, working, arguments] : Settings::start_programs_path)
            create_process(path, working, arguments);
        started_programs = true;
        std::thread(
            []()
            {
                api->Renderer.Deregister(addon_render);
                api->Log(ELogLevel_INFO, addon_name, "launched every program & deregistered renderer!");
            })
            .detach();
    }
}

void addon_options()
{
    display_kill_processes_on_close_option();
    if (ImGui::CollapsingHeader("Programs to start on game start")) {
        display_start_programs_option();
    }
    if (ImGui::CollapsingHeader("Programs to start on game close")) {
        display_exit_programs_option();
    }
    if (ImGui::CollapsingHeader("Add program##AppLauncherAddProgramHeader")) {
        display_add_program_option();
    }
}
