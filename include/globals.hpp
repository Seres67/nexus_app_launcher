//
// Created by Seres67 on 06/08/2024.
//

#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <nexus/Nexus.h>
#include <string>
#include <vector>

// handle to self hmodule
extern HMODULE hSelf;
// addon definition
extern AddonDefinition AddonDef;
// addon api
extern AddonAPI *API;
extern HWND game_handle;
// if all programs have been started
extern bool startedPrograms;
// supported extensions
extern std::vector<std::string> supported_extensions;
// Path environment variable
extern char path[10240];
// program path when adding a program
extern char newProgram[256];
// program arguments when adding a program
extern char newArguments[256];
// whether to start the program on game exit or not
extern bool start_on_exit;
// which index of Settings::startProgramsPath to edit
extern int editProgram;
// program path when editing a program
extern char editProgramPath[256];
// program arguments when editing a program
extern char editProgramArguments[256];
// which index of Settings::exitProgramsPath to edit
extern int editExitProgram;
// whether the program is valid or not
extern bool isProgramValid;

typedef struct
{
    PROCESS_INFORMATION pi;
    STARTUPINFOA si;
} process;
// list of processes
extern std::vector<process> processes;

#endif // GLOBALS_HPP