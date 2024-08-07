//
// Created by Seres67 on 06/08/2024.
//

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <mutex>
#include <nlohmann/json.hpp>

namespace Settings
{

void Load(const std::filesystem::path &aPath);
void Save(const std::filesystem::path &aPath);

extern nlohmann::json json_settings;
extern std::filesystem::path SettingsPath;
extern std::mutex mutex;
extern bool IsAddonEnabled;
extern bool KillProcessesOnClose;

typedef struct
{
    std::string path;
    std::string arguments;
} program;

void from_json(const nlohmann::json &j, program &p);

void to_json(nlohmann::json &j, const program &p);

void add_program(const std::string &program, const std::string &arguments);
void remove_program(int i);

extern std::vector<program> programsPath;

extern const char *IS_ADDON_ENABLED;
extern const char *KILL_PROCESSES_ON_CLOSE;
extern const char *PROGRAMS_PATH;
} // namespace Settings

#endif // SETTINGS_HPP
