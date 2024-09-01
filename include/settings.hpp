#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <mutex>
#include <nlohmann/json.hpp>

namespace Settings
{

void load(const std::filesystem::path &path);
void save(const std::filesystem::path &path);

extern nlohmann::json json_settings;
extern std::filesystem::path settings_path;
extern std::mutex mutex;
extern bool is_addon_enabled;
extern bool kill_processes_on_close;

typedef struct
{
    std::string path;
    std::string arguments;
} program;

void from_json(const nlohmann::json &j, program &p);

void to_json(nlohmann::json &j, const program &p);

void add_start_program(const std::string &program, const std::string &arguments);
void add_exit_program(const std::string &program, const std::string &arguments);
void remove_start_program(int i);
void remove_exit_program(int i);

extern std::vector<program> start_programs_path;
extern std::vector<program> exit_programs_path;

extern const char *IS_ADDON_ENABLED;
extern const char *KILL_PROCESSES_ON_CLOSE;
extern const char *START_PROGRAMS_PATH;
extern const char *EXIT_PROGRAMS_PATH;
} // namespace Settings

#endif // SETTINGS_HPP
