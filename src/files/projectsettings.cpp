#include "projectsettings.h"
#include "core/meditlog.h"
#include "text/startswith.h"
#include <filesystem>
#include <json/json.h>

namespace {

std::string translateInclude(std::string flag,
                             const std::filesystem::path &root) {
    if (starts_with(flag, "-I") && flag.size() > 2) {
        if (flag.at(2) != '/') {
            flag = "-I" + (root / flag.substr(2)).string();
        }
    }

    return flag;
}
} // namespace

void ProjectSettings::load(std::filesystem::path projectFile) {
    if (!std::filesystem::exists(projectFile)) {
        return;
    }

    auto json = Json{};
    try {
        std::fstream(projectFile) >> json;
    }
    catch (Json::ParsingError &error) {
        logStatusMessage(error.what());
    }

    // TODO: Add more multiple build configuration
    if (auto it = json.find("build"); it != json.end()) {
        buildCommand = it->value;
    }

    // TODO: Add more multiple build configuration
    if (auto it = json.find("run"); it != json.end()) {
        runCommand = it->value;
    }

    if (auto it = json.find("flags"); it != json.end()) {
        flags.clear();

        if (it->type == Json::String) {
            // auto ss = std::istringstream();
            // flags = std::vector<std::string>{};
            // for (std::string s; ss >> s;) {
            //     flags.push_back(s);
            // }
            flags.push_back(it->value);
        }
        else if (it->type == Json::Array) {
            flags.clear();
            for (auto &value : *it) {
                if (value.type == Json::String) {
                    flags.push_back(translateInclude(value.value, root));
                }
            }
        }
    }

    if (auto it = json.find("debug"); it != json.end()) {
        if (auto command = it->find("command"); command != it->end()) {
            debug.command = command->string();
        }
        if (auto dir = it->find("working_dir"); dir != it->end()) {
            debug.workingDir = dir->string();
        }
    }
}
