#pragma once

#include "filesystem.h"
#include <vector>

class ProjectFiles {
public:
    filesystem::path root(filesystem::path);

    std::vector<filesystem::path> allProjectFiles(
        const filesystem::path &pathInProject, size_t max = 20000);
};
