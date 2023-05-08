#pragma once

#include "script/ienvironment.h"
#include "text/position.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// TODO: Reuse for other refactorizations
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
