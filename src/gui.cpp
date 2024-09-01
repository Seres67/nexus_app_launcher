#include "gui.hpp"
#include <globals.hpp>
#include <imgui/imgui.h>
#include <settings.hpp>

void display_active_option()
{
    if (ImGui::Checkbox("Enabled##AppLauncherEnabled", &Settings::is_addon_enabled)) {
        Settings::json_settings[Settings::IS_ADDON_ENABLED] = Settings::is_addon_enabled;
        Settings::save(Settings::settings_path);
    }
}

void display_kill_processes_on_close_option()
{
    if (ImGui::Checkbox("Kill Processes On Close##Widget", &Settings::kill_processes_on_close)) {
        Settings::json_settings[Settings::KILL_PROCESSES_ON_CLOSE] = Settings::kill_processes_on_close;
        Settings::save(Settings::settings_path);
    }
}

void kill_process(const int i)
{
    TerminateProcess(processes[i].pi.hProcess, 0);
    WaitForSingleObject(processes[i].pi.hProcess, INFINITE);
    CloseHandle(processes[i].pi.hProcess);
    CloseHandle(processes[i].pi.hThread);
    processes.erase(processes.begin() + i);
}

void display_start_programs_option()
{
    ImGui::Text("Start Programs");
    for (auto i = 0; i < Settings::start_programs_path.size(); i++) {
        ImGui::PushID(i);
        if (edit_program == i) {
            ImGui::Checkbox("Start on game exit", &edit_start_on_exit);
            ImGui::InputText("Program Path##ProgramPathInput", edit_program_path, 256);
            ImGui::InputText("Program Arguments##ProgramArgumentsInput", edit_program_arguments, 256);
            if (ImGui::Button("Confirm##ConfirmButton")) {
                if (edit_start_on_exit) {
                    program_to_swap = i;
                }
                Settings::start_programs_path[i].path = edit_program_path;
                Settings::start_programs_path[i].arguments = edit_program_arguments;
                Settings::json_settings[Settings::START_PROGRAMS_PATH] = Settings::start_programs_path;
                Settings::save(Settings::settings_path);
                memset(edit_program_path, 0, 256);
                memset(edit_program_arguments, 0, 256);
                is_program_valid = true;
                edit_program = -1;
                if (Settings::kill_processes_on_close)
                    kill_process(i);
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##CancelButton")) {
                memset(edit_program_path, 0, 256);
                memset(edit_program_arguments, 0, 256);
                is_program_valid = true;
                edit_program = -1;
            }
        } else {
            if (std::filesystem::exists(Settings::start_programs_path[i].path)) {
                ImGui::Text("%s", Settings::start_programs_path[i].path.c_str());
                ImGui::SameLine();
                ImGui::Text("%s", Settings::start_programs_path[i].arguments.c_str());
            } else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", Settings::start_programs_path[i].path.c_str());
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Path does not exist.");
                }
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", Settings::start_programs_path[i].arguments.c_str());
            }
        }
        ImGui::SameLine();
        if (edit_program == -1 && ImGui::Button("Edit")) {
            edit_program = i;
            strcpy_s(edit_program_path, Settings::start_programs_path[i].path.c_str());
            strcpy_s(edit_program_arguments, Settings::start_programs_path[i].arguments.c_str());
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete")) {
            if (Settings::kill_processes_on_close && processes.size() > i)
                kill_process(i);
            Settings::remove_start_program(i);
            edit_program = -1;
            continue;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Remove program from list and kills it if kill processes on close is enabled.");
        }
        ImGui::PopID();
    }
    if (program_to_swap != -1) {
        std::string path(Settings::start_programs_path[program_to_swap].path);
        std::string arguments(Settings::start_programs_path[program_to_swap].arguments);
        kill_process(program_to_swap);
        Settings::remove_start_program(program_to_swap);
        Settings::add_exit_program(path, arguments);
        program_to_swap = -1;
        edit_start_on_exit = false;
    }
}

void display_exit_programs_option()
{
    ImGui::Text("Exit Programs");
    for (auto i = 0; i < Settings::exit_programs_path.size(); i++) {
        ImGui::PushID(("exit" + std::to_string(i)).c_str());
        if (edit_exit_program == i) {
            ImGui::Checkbox("Start on game exit", &edit_exit_start_on_exit);
            ImGui::InputText("Program Path##ProgramPathInput", edit_program_path, 256);
            ImGui::InputText("Program Arguments##ProgramArgumentsInput", edit_program_arguments, 256);
            if (ImGui::Button("Confirm##ConfirmButton")) {
                if (!edit_exit_start_on_exit) {
                    program_to_swap = i;
                }
                Settings::exit_programs_path[i].path = edit_program_path;
                Settings::exit_programs_path[i].arguments = edit_program_arguments;
                Settings::json_settings[Settings::EXIT_PROGRAMS_PATH] = Settings::exit_programs_path;
                Settings::save(Settings::settings_path);
                memset(edit_program_path, 0, 256);
                memset(edit_program_arguments, 0, 256);
                is_program_valid = true;
                edit_exit_program = -1;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##CancelButton")) {
                memset(edit_program_path, 0, 256);
                memset(edit_program_arguments, 0, 256);
                is_program_valid = true;
                edit_exit_program = -1;
            }
        } else {
            if (std::filesystem::exists(Settings::exit_programs_path[i].path)) {
                ImGui::Text("%s", Settings::exit_programs_path[i].path.c_str());
                ImGui::SameLine();
                ImGui::Text("%s", Settings::exit_programs_path[i].arguments.c_str());
            } else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", Settings::exit_programs_path[i].path.c_str());
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Path does not exist.");
                }
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", Settings::exit_programs_path[i].arguments.c_str());
            }
        }
        ImGui::SameLine();
        if (edit_exit_program == -1 && ImGui::Button("Edit")) {
            edit_exit_program = i;
            strcpy_s(edit_program_path, Settings::exit_programs_path[i].path.c_str());
            strcpy_s(edit_program_arguments, Settings::exit_programs_path[i].arguments.c_str());
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete")) {
            Settings::remove_exit_program(i);
            edit_exit_program = -1;
            continue;
        }
        ImGui::PopID();
    }
    if (program_to_swap != -1) {
        std::string path(Settings::exit_programs_path[program_to_swap].path);
        std::string arguments(Settings::exit_programs_path[program_to_swap].arguments);
        Settings::remove_exit_program(program_to_swap);
        Settings::add_start_program(path, arguments);
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
        api->Log(ELogLevel_DEBUG, "App Launcher", log);
        if (std::filesystem::exists(p)) {
            return p;
        }
    }
    return program;
}

#include <imgui-filebrowser/imfilebrowser.h>
ImGui::FileBrowser file_browser;
void display_add_program_option()
{
    if (ImGui::Button("Open File Picker##OpenFilePickerButton")) {
        file_browser.SetTypeFilters(supported_extensions);
        file_browser.Open();
    }
    file_browser.Display();
    if (file_browser.HasSelected()) {
        strcpy_s(new_program, file_browser.GetSelected().string().c_str());
    }
    ImGui::NewLine();
    ImGui::Checkbox("Start program on game exit", &start_on_exit);
    ImGui::InputText("Program Path##ProgramPathInput", new_program, 256);
    ImGui::InputText("Program Arguments##ProgramArgumentsInput", new_arguments, 256);
    if (ImGui::Button("Add program##AddProgramButton")) {
        std::string program(new_program);
        if (program.empty())
            return;
        const std::string arguments(new_arguments);
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
                api->Log(ELogLevel_DEBUG, "App Launcher", "Trying to find program in PATH...");
                program = get_program_path(program);
            }
            if (start_on_exit) {
                Settings::add_exit_program(program, arguments);
            } else {
                Settings::add_start_program(program, arguments);
            }
            file_browser.ClearSelected();
            memset(new_program, 0, 256);
            memset(new_arguments, 0, 256);
            is_program_valid = true;
        }
    }
    if (!is_program_valid) {
        ImGui::SameLine();
        ImGui::Text("Path must lead to an executable file. (.exe)");
    }
}