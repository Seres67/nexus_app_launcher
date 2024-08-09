//
// Created by Seres67 on 06/08/2024.
//
#include "globals.hpp"

HMODULE self_module = nullptr;
AddonDefinition addon_def{};
AddonAPI *API = nullptr;
HWND game_handle = nullptr;
bool started_programs = false;
std::vector<std::string> supported_extensions = {".exe", ".EXE", ".bat", ".BAT", ".cmd", ".CMD"};
char path[10240];
char new_program[256] = {};
char new_arguments[256] = {};
bool start_on_exit = false;
int edit_program = -1;
char edit_program_path[256] = {};
char edit_program_arguments[256] = {};
int edit_exit_program = -1;
bool is_program_valid = true;
std::vector<process> processes;