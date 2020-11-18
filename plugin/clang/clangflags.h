#pragma once

#include "files/project.h"
#include <string>
#include <vector>

struct ClangFlags {
    std::vector<std::string> includes;
    std::vector<const char *> args;
    ClangFlags(Project &project) {

        auto ss = std::istringstream(project.settings().flags);
        includes = std::vector<std::string>{};

        for (std::string s; ss >> s;) {
            includes.push_back(s);
        }
        for (auto &i : includes) {
            args.push_back(i.c_str());
        }
    }
};
