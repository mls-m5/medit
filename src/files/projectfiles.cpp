
#include "projectfiles.h"
#include "files/extensions.h"
#include "json/json.h"
#include <fstream>
#include <string_view>

namespace {

const std::string_view projectFileName = ".medit.json";

}

filesystem::path ProjectFiles::root(filesystem::path path) const {
    path = filesystem::absolute(path);

    do {

        if (filesystem::exists(path / projectFileName)) {
            return path;
        }

        if (filesystem::exists(path / "/.git")) {
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
    loadProjectFile();
}

std::vector<filesystem::path> ProjectFiles::findProjectFiles(
    const filesystem::path &pathInProject, size_t max) {
    auto &root = _settings.root;
    root = this->root(pathInProject);

    if (root.empty()) {
        return {};
    }

    std::vector<filesystem::path> paths;

    size_t i = 0;
    for (auto path : filesystem::recursive_directory_iterator(root)) {
        if (!isCpp(path) && !isJson(path)) {
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

void ProjectFiles::loadProjectFile() {
    auto projectFile = _settings.root / projectFileName;
    if (!filesystem::exists(projectFile)) {
        return;
    }

    auto json = Json{};
    std::fstream(projectFile) >> json;

    auto it = json.find("build");
    if (it != json.end()) {
        _settings.buildCommand = it->value;
    }
}
