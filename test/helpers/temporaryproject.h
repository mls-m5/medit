#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace test_helpers {

// ProjectSettings writes project history under HOME. Keep model and watcher
// tests isolated from both the real configuration and the repository files.
struct TemporaryProject {
    std::optional<std::string> previousHome;
    std::filesystem::path directory;
    std::filesystem::path root;

    TemporaryProject() {
        if (const auto *home = std::getenv("HOME"))
            previousHome = home;
        auto pattern = (std::filesystem::temp_directory_path() /
                        "medit-project-test-XXXXXX")
                           .string();
        const auto *created = mkdtemp(pattern.data());
        if (!created)
            throw std::runtime_error("Cannot create temporary project");
        directory = created;
        root = directory / "project";
        std::filesystem::create_directory(root);
        write(".medit.json", R"({"build":"matmake"})");
        setenv("HOME", directory.c_str(), 1);
    }

    ~TemporaryProject() {
        if (previousHome)
            setenv("HOME", previousHome->c_str(), 1);
        else
            unsetenv("HOME");
        std::error_code error;
        std::filesystem::remove_all(directory, error);
    }

    std::filesystem::path path(const std::filesystem::path &relative) const {
        return (root / relative).lexically_normal();
    }

    void write(const std::filesystem::path &relative,
               std::string_view text = "") {
        const auto file = path(relative);
        std::filesystem::create_directories(file.parent_path());
        std::ofstream{file} << text;
    }
};

} // namespace test_helpers
