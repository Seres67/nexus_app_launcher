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
std::vector<program> start_programs_path;
std::vector<program> exit_programs_path;
std::filesystem::path settings_path;
bool is_addon_enabled = true;
bool kill_processes_on_close = false;

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
    start_programs_path.emplace_back(program, arguments);
    json_settings[START_PROGRAMS_PATH] = start_programs_path;
    save(settings_path);
}

void add_exit_program(const std::string &program, const std::string &arguments)
{
    exit_programs_path.emplace_back(program, arguments);
    json_settings[EXIT_PROGRAMS_PATH] = exit_programs_path;
    save(settings_path);
}

void remove_start_program(const int i)
{
    start_programs_path.erase(start_programs_path.begin() + i);
    json_settings[START_PROGRAMS_PATH] = start_programs_path;
    save(settings_path);
}

void remove_exit_program(const int i)
{
    exit_programs_path.erase(exit_programs_path.begin() + i);
    json_settings[EXIT_PROGRAMS_PATH] = exit_programs_path;
    save(settings_path);
}

void load(const std::filesystem::path &path)
{
    json_settings = json::object();
    if (!std::filesystem::exists(path)) {
        return;
    }

    {
        std::lock_guard lock(mutex);
        try {
            if (std::ifstream file(path); file.is_open()) {
                json_settings = json::parse(file);
                file.close();
            }
        } catch (json::parse_error &ex) {
            API->Log(ELogLevel_WARNING, "App Launcher", "settings.json could not be parsed.");
            API->Log(ELogLevel_WARNING, "App Launcher", ex.what());
        }
    }
    if (!json_settings[IS_ADDON_ENABLED].is_null()) {
        json_settings[IS_ADDON_ENABLED].get_to(is_addon_enabled);
    }
    if (!json_settings[KILL_PROCESSES_ON_CLOSE].is_null()) {
        json_settings[KILL_PROCESSES_ON_CLOSE].get_to(kill_processes_on_close);
    }
    if (!json_settings[START_PROGRAMS_PATH].is_null()) {
        json_settings[START_PROGRAMS_PATH].get_to(start_programs_path);
    }
    if (!json_settings[EXIT_PROGRAMS_PATH].is_null()) {
        json_settings[EXIT_PROGRAMS_PATH].get_to(exit_programs_path);
    }
    API->Log(ELogLevel_INFO, "App Launcher", "settings loaded!");
}

void save(const std::filesystem::path &path)
{
    if (json_settings.is_null()) {
        API->Log(ELogLevel_WARNING, "App Launcher", "settings.json is null, cannot save.");
        return;
    }
    if (!std::filesystem::exists(path.parent_path())) {
        std::filesystem::create_directories(path.parent_path());
    }
    {
        std::lock_guard lock(mutex);
        if (std::ofstream file(path); file.is_open()) {
            file << json_settings.dump(1, '\t') << std::endl;
            file.close();
        }
        API->Log(ELogLevel_INFO, "App Launcher", "settings saved!");
    }
}
} // namespace Settings