#include "globals.hpp"

const char addon_name[] = "App Launcher";
HMODULE self_module = nullptr;
AddonDefinition addon_def{};
AddonAPI *api = nullptr;
HWND game_handle = nullptr;
bool started_programs = false;
std::vector<std::string> supported_extensions = {".exe", ".EXE", ".bat", ".BAT", ".cmd", ".CMD"};
char path[4096];
char new_program[256] = {};
char new_program_working_directory[256] = {};
char new_arguments[256] = {};
bool start_on_exit = false;
int edit_program = -1;
char edit_program_path[256] = {};
char edit_program_working_dir[256] = {};
char edit_program_arguments[256] = {};
int edit_exit_program = -1;
bool edit_start_on_exit = false;
bool edit_exit_start_on_exit = false;
int program_to_swap = -1;
bool is_program_valid = true;
std::vector<process> processes;

void kill_process(const int i)
{
    if (i >= processes.size())
        return;
    TerminateProcess(processes[i].pi.hProcess, 0);
    WaitForSingleObject(processes[i].pi.hProcess, INFINITE);
    CloseHandle(processes[i].pi.hProcess);
    CloseHandle(processes[i].pi.hThread);
    processes.erase(processes.begin() + i);
}
