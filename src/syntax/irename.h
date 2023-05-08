#pragma once

#include "script/ienvironment.h"
#include "text/position.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// TODO: Move to own file along with `Changes`
#include <istream>
#include <ostream>
#include <regex>

// TODO: Reuse for other refactorizations
/// Struct that represent changes that could to be made
/// The user will be able to select which to use
struct Changes {
    struct Change {
        Position begin;
        Position end;
        std::string newText;
    };

    struct FileChanges {
        std::filesystem::path file;
        std::vector<Change> changes;
    };

    std::vector<FileChanges> changes;

    void serialize(std::ostream &out) const {
        for (const auto &fileChanges : changes) {
            for (const auto &change : fileChanges.changes) {
                out << fileChanges.file.string() << ":" << change.begin.y() + 1
                    << ":" << change.begin.x() + 1 << "-" << change.end.y() + 1
                    << ":" << change.end.x() + 1 << " " << change.newText
                    << std::endl;
            }
        }
    }

    FileChanges &operator[](const std::filesystem::path &path) {
        auto it = std::find_if(changes.begin(),
                               changes.end(),
                               [&](const FileChanges &fileChanges) {
                                   return fileChanges.file == path;
                               });
        if (it != changes.end()) {
            return *it;
        }
        else {
            changes.push_back({path, {}});
            return changes.back();
        }
    }

    void deserialize(std::istream &in) {
        changes.clear();
        std::string line;
        std::regex line_regex(R"((.+):(\d+):(\d+)-(\d+):(\d+)\s+(.+))");
        std::smatch line_match;

        while (std::getline(in, line)) {
            if (std::regex_match(line, line_match, line_regex)) {
                std::filesystem::path filePath = line_match[1].str();
                Position begin(std::stoi(line_match[3].str()) - 1,
                               std::stoi(line_match[2].str()) - 1);
                Position end(std::stoi(line_match[5].str()) - 1,
                             std::stoi(line_match[4].str()) - 1);

                std::string newText = line_match[6].str();

                FileChanges &fileChanges = (*this)[filePath];
                fileChanges.changes.push_back({begin, end, newText});
            }
        }
    }
    /// Sort in reverse order to make ready to apply
    void sort() {
        for (auto &fileChanges : changes) {
            std::sort(fileChanges.changes.rbegin(),
                      fileChanges.changes.rend(),
                      [](const Change &a, const Change &b) {
                          if (a.begin.y() == b.begin.y()) {
                              return a.begin.x() < b.begin.x();
                          }
                          else {
                              return a.begin.y() < b.begin.y();
                          }
                      });
        }
    }
};

class IRename {
public:
    struct RenameArgs {
        std::string newName;
    };

    struct PrepareCallbackArgs {
        Position start;
        Position end;
    };

    virtual bool doesSupportPrepapre() = 0;

    virtual bool prepare(std::shared_ptr<IEnvironment> env,
                         std::function<void(PrepareCallbackArgs)>) = 0;

    virtual bool rename(std::shared_ptr<IEnvironment>,
                        RenameArgs,
                        std::function<void(Changes)> callback) = 0;

    virtual bool shouldEnable(std::filesystem::path) const = 0;

    virtual int priority() const = 0;
};
