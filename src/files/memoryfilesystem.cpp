#include "files/memoryfilesystem.h"
#include <algorithm>

namespace {

Path normal(const Path &path) {
    return path.lexically_normal();
}

/// Whether `path` is `ancestor` or below it.
bool isBelow(const Path &path, const Path &ancestor) {
    auto [a, p] = std::mismatch(
        ancestor.begin(), ancestor.end(), path.begin(), path.end());
    return a == ancestor.end() ||
           (a->empty() && std::next(a) == ancestor.end());
}

} // namespace

std::optional<std::string> MemoryFileSystem::read(const Path &path) {
    auto lock = std::lock_guard{_mutex};
    auto it = _files.find(normal(path));
    if (it == _files.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool MemoryFileSystem::write(const Path &path, std::string_view content) {
    auto lock = std::lock_guard{_mutex};
    const auto p = normal(path);
    if (isDirectory(p)) {
        return false;
    }
    _files[p] = std::string{content};
    return true;
}

bool MemoryFileSystem::rename(const Path &from, const Path &to) {
    auto lock = std::lock_guard{_mutex};
    const auto f = normal(from);
    const auto t = normal(to);
    auto it = _files.find(f);
    if (it == _files.end() || _files.contains(t) || isDirectory(t)) {
        return false;
    }
    _files[t] = std::move(it->second);
    _files.erase(f);
    return true;
}

FileInfo MemoryFileSystem::stat(const Path &path) {
    auto lock = std::lock_guard{_mutex};
    const auto p = normal(path);
    auto info = FileInfo{};
    if (_files.contains(p)) {
        info.exists = true;
    }
    else if (isDirectory(p)) {
        info.exists = true;
        info.isDirectory = true;
    }
    return info;
}

std::vector<Path> MemoryFileSystem::list(const Path &root,
                                         const ListFilter &filter) {
    auto lock = std::lock_guard{_mutex};
    const auto r = normal(root);
    auto ret = std::vector<Path>{};
    for (auto &[path, content] : _files) {
        if (ret.size() >= filter.limit) {
            break;
        }
        if (path == r || !isBelow(path, r)) {
            continue;
        }
        // Every directory between the root and the file must be entered.
        auto entered = true;
        if (filter.enterDirectory) {
            for (auto dir = path.parent_path(); dir != r && isBelow(dir, r);
                 dir = dir.parent_path()) {
                if (!filter.enterDirectory(dir)) {
                    entered = false;
                    break;
                }
            }
        }
        if (entered && (!filter.includeFile || filter.includeFile(path))) {
            ret.push_back(path);
        }
    }
    return ret;
}

void MemoryFileSystem::directory(const Path &path) {
    auto lock = std::lock_guard{_mutex};
    _directories[normal(path)] = true;
}

void MemoryFileSystem::remove(const Path &path) {
    auto lock = std::lock_guard{_mutex};
    _files.erase(normal(path));
    _directories.erase(normal(path));
}

bool MemoryFileSystem::isDirectory(const Path &path) const {
    if (_directories.contains(path)) {
        return true;
    }
    for (auto &[file, content] : _files) {
        if (file != path && isBelow(file, path)) {
            return true;
        }
    }
    return false;
}
