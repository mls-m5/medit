#pragma once

#include "files/project.h"
#include <string>
#include <vector>

struct ClangFlags {
    std::vector<std::string> includes;
    std::vector<const char *> args;
    ClangFlags(Project &project)
        : includes(project.settings().flags) {
        for (auto &i : includes) {
            args.push_back(i.c_str());
        }
    }
};
