#pragma once

#include "meditfwd.h"
#include "text/position.h"
#include <filesystem>
#include <iosfwd>
#include <istream>
#include <vector>

/// Struct that represent changes that could to be made
/// The user will be able to select which to use
struct Changes {
    struct Change {
        Position begin;
        Position end;
        std::string newText;

        void apply(Buffer &buffer) const;
    };

    struct FileChanges {
        std::filesystem::path file;
        std::vector<Change> changes;
    };

    std::vector<FileChanges> changes;

    void serialize(std::ostream &out) const;

    FileChanges &operator[](const std::filesystem::path &path);

    void deserialize(std::istream &in);
    /// Sort in reverse order to make ready to apply
    void sort();

    /// Apply changes
    /// Also "consumes" the changes so when they are applied none is
    /// left on the object
    void apply(IEnvironment &env);
};
