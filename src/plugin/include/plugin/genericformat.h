#pragma once

#include "core/icommandrunner.h"
#include "syntax/iformat.h"
#include <filesystem>
#include <optional>

// Format file with a installed command line tool
class GenericFormat : public IFormat {
public:
    explicit GenericFormat(ICommandRunner &runner)
        : _runner{runner} {}

    //! @see IFormat
    bool format(Editor &) override;

    /// The command that formats the file at `path` (absolute) in place, or
    /// nothing if no installed tool handles it.
    static std::optional<Command> formatCommand(
        const std::filesystem::path &path);

private:
    ICommandRunner &_runner;
};
