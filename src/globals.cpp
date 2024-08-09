//
// Created by Seres67 on 06/08/2024.
//
#include "globals.hpp"

HMODULE hSelf = nullptr;
AddonDefinition AddonDef{};
AddonAPI *API = nullptr;
HWND game_handle = nullptr;
bool startedPrograms = false;
std::vector<std::string> supported_extensions = {".exe", ".EXE", ".bat", ".BAT", ".cmd", ".CMD"};
char path[10240];
char newProgram[256] = {};
char newArguments[256] = {};
bool start_on_exit = false;
int editProgram = -1;
char editProgramPath[256] = {};
char editProgramArguments[256] = {};
int editExitProgram = -1;
bool isProgramValid = true;
std::vector<process> processes;