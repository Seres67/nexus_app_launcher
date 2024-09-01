#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <nexus/Nexus.h>
#include <string>
#include <vector>

// handle to self hmodule
extern HMODULE self_module;
// addon definition
extern AddonDefinition addon_def;
// addon api
extern AddonAPI *api;
extern HWND game_handle;
// if all programs have been started
extern bool started_programs;
// supported extensions
extern std::vector<std::string> supported_extensions;
// Path environment variable
extern char path[10240];
// program path when adding a program
extern char new_program[256];
// program arguments when adding a program
extern char new_arguments[256];
// whether to start the program on game exit or not
extern bool start_on_exit;
// which index of Settings::startProgramsPath to edit
extern int edit_program;
// program path when editing a program
extern char edit_program_path[256];
// program arguments when editing a program
extern char edit_program_arguments[256];
// which index of Settings::exitProgramsPath to edit
extern int edit_exit_program;
// whether to start the program on game exit or not when editing a program
extern bool edit_start_on_exit;
extern bool edit_exit_start_on_exit;
extern int program_to_swap;
// whether the program is valid or not
extern bool is_program_valid;

typedef struct
{
    PROCESS_INFORMATION pi;
    STARTUPINFOA si;
} process;
// list of processes
extern std::vector<process> processes;

#endif // GLOBALS_HPP