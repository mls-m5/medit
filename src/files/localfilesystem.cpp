#include "files/ifilesystem.h"
#include <fstream>
#include <iterator>
#include <system_error>

namespace {

class LocalFileSystem : public IFileSystem {
public:
    std::optional<std::string> read(const Path &path) override {
        auto error = std::error_code{};
        if (!std::filesystem::is_regular_file(path, error)) {
            return std::nullopt;
        }
        auto file = std::ifstream{path, std::ios::binary};
        if (!file.is_open()) {
            return std::nullopt;
        }
        auto bytes = std::string{std::istreambuf_iterator<char>{file},
                                 std::istreambuf_iterator<char>{}};
        if (file.bad()) {
            return std::nullopt;
        }
        return bytes;
    }

    bool write(const Path &path, std::string_view content) override {
        auto error = std::error_code{};
        if (std::filesystem::is_directory(path, error)) {
            return false;
        }
        auto file = std::ofstream{path, std::ios::binary};
        file.write(content.data(),
                   static_cast<std::streamsize>(content.size()));
        return static_cast<bool>(file);
    }

    bool rename(const Path &from, const Path &to) override {
        auto error = std::error_code{};
        if (std::filesystem::exists(
                std::filesystem::symlink_status(to, error))) {
            return false;
        }
        std::filesystem::rename(from, to, error);
        return !error;
    }

    FileInfo stat(const Path &path) override {
        auto error = std::error_code{};
        const auto link = std::filesystem::symlink_status(path, error);
        auto info = FileInfo{};
        info.isSymlink = std::filesystem::is_symlink(link);
        const auto status = std::filesystem::status(path, error);
        info.exists = std::filesystem::exists(status);
        info.isDirectory = std::filesystem::is_directory(status);
        return info;
    }

    std::vector<Path> list(const Path &root,
                           const ListFilter &filter) override {
        auto ret = std::vector<Path>{};
        auto error = std::error_code{};
        auto it = std::filesystem::recursive_directory_iterator{
            root,
            std::filesystem::directory_options::skip_permission_denied,
            error};
        const auto end = std::filesystem::recursive_directory_iterator{};
        for (; !error && it != end && ret.size() < filter.limit;
             it.increment(error)) {
            const auto &entry = *it;
            auto entryError = std::error_code{};
            if (entry.is_directory(entryError)) {
                // The iterator does not follow directory symlinks by default.
                if (filter.enterDirectory &&
                    !filter.enterDirectory(entry.path())) {
                    it.disable_recursion_pending();
                }
                continue;
            }
            if (!entry.is_regular_file(entryError)) {
                continue;
            }
            if (filter.includeFile && !filter.includeFile(entry.path())) {
                continue;
            }
            ret.push_back(entry.path());
        }
        return ret;
    }
};

} // namespace

IFileSystem &localFileSystem() {
    static auto fileSystem = LocalFileSystem{};
    return fileSystem;
}
