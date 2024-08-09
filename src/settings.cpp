//
// Created by Seres67 on 06/08/2024.
//

#include "settings.hpp"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <fstream>
#include <globals.hpp>

#include <nexus/Nexus.h>

using json = nlohmann::json;
namespace Settings
{
const char *IS_ADDON_ENABLED = "IsAddonEnabled";
const char *KILL_PROCESSES_ON_CLOSE = "KillProcessesOnClose";
const char *START_PROGRAMS_PATH = "StartProgramsPath";
const char *EXIT_PROGRAMS_PATH = "ExitProgramsPath";

json json_settings;
std::mutex mutex;
std::vector<program> startProgramsPath;
std::vector<program> exitProgramsPath;
std::filesystem::path SettingsPath;
bool IsAddonEnabled = true;
bool KillProcessesOnClose = false;

void from_json(const json &j, program &p)
{
    j.at("path").get_to(p.path);
    j.at("arguments").get_to(p.arguments);
}

void to_json(json &j, const program &p)
{
    j["path"] = p.path;
    j["arguments"] = p.arguments;
}

void add_start_program(const std::string &program, const std::string &arguments)
{
    startProgramsPath.emplace_back(program, arguments);
    json_settings[START_PROGRAMS_PATH] = startProgramsPath;
    Save(SettingsPath);
}

void add_exit_program(const std::string &program, const std::string &arguments)
{
    exitProgramsPath.emplace_back(program, arguments);
    json_settings[EXIT_PROGRAMS_PATH] = exitProgramsPath;
    Save(SettingsPath);
}

void remove_start_program(const int i)
{
    startProgramsPath.erase(startProgramsPath.begin() + i);
    json_settings[START_PROGRAMS_PATH] = startProgramsPath;
    Save(SettingsPath);
}

void remove_exit_program(const int i)
{
    exitProgramsPath.erase(exitProgramsPath.begin() + i);
    json_settings[EXIT_PROGRAMS_PATH] = exitProgramsPath;
    Save(SettingsPath);
}

void Load(const std::filesystem::path &aPath)
{
    json_settings = json::object();
    if (!std::filesystem::exists(aPath)) {
        return;
    }

    {
        std::lock_guard lock(mutex);
        try {
            if (std::ifstream file(aPath); file.is_open()) {
                json_settings = json::parse(file);
                file.close();
            }
        } catch (json::parse_error &ex) {
            API->Log(ELogLevel_WARNING, "App Launcher", "settings.json could not be parsed.");
            API->Log(ELogLevel_WARNING, "App Launcher", ex.what());
        }
    }
    if (!json_settings[IS_ADDON_ENABLED].is_null()) {
        json_settings[IS_ADDON_ENABLED].get_to(IsAddonEnabled);
    }
    if (!json_settings[KILL_PROCESSES_ON_CLOSE].is_null()) {
        json_settings[KILL_PROCESSES_ON_CLOSE].get_to(KillProcessesOnClose);
    }
    if (!json_settings[START_PROGRAMS_PATH].is_null()) {
        json_settings[START_PROGRAMS_PATH].get_to(startProgramsPath);
    }
    if (!json_settings[EXIT_PROGRAMS_PATH].is_null()) {
        json_settings[EXIT_PROGRAMS_PATH].get_to(exitProgramsPath);
    }
    API->Log(ELogLevel_INFO, "App Launcher", "settings loaded!");
}

void Save(const std::filesystem::path &aPath)
{
    if (json_settings.is_null()) {
        API->Log(ELogLevel_WARNING, "App Launcher", "settings.json is null, cannot save.");
        return;
    }
    if (!std::filesystem::exists(aPath.parent_path())) {
        std::filesystem::create_directories(aPath.parent_path());
    }
    {
        std::lock_guard lock(mutex);
        if (std::ofstream file(aPath); file.is_open()) {
            file << json_settings.dump(1, '\t') << std::endl;
            file.close();
        }
        API->Log(ELogLevel_INFO, "App Launcher", "settings saved!");
    }
}
} // namespace Settings