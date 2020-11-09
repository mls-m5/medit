
#include "projectfiles.h"

filesystem::path ProjectFiles::root(filesystem::path path) const {
    path = filesystem::absolute(path);
    do {
        // Just a fast solution
        auto projectRootPath = path;
        projectRootPath /= "/.git";

        if (filesystem::exists(projectRootPath)) {
            return path;
        }

        auto newPath = filesystem::path("");
        for (auto it = path.begin(); it != --path.end(); ++it) {
            newPath /= it->string();
        }

        path = newPath;

    } while (!path.empty());

    return {};
}

void ProjectFiles::updateCache(const filesystem::path &pathInProject,
                               size_t max) {
    _fileCache = findProjectFiles(pathInProject, max);
}

std::vector<filesystem::path> ProjectFiles::findProjectFiles(
    const filesystem::path &pathInProject, size_t max) const {
    auto root = this->root(pathInProject);

    if (root.empty()) {
        return {};
    }
    std::vector<filesystem::path> paths;

    size_t i = 0;
    for (auto path : filesystem::recursive_directory_iterator(root)) {
        if (path.path().extension() != ".h" &&
            path.path().extension() != ".cpp") {
            continue;
        }
        paths.push_back(path);
        ++i;
        if (i > max) {
            return paths;
        }
    }

    return paths;
}
