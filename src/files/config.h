#pragma once

#include <filesystem>
#include <optional>

/// Find or create a directory for medit files
std::optional<std::filesystem::path> localConfigDirectory(
    std::filesystem::path relativePath = {}, bool createIfNonexistent = false);

/// Look for files in some directory medit looks in
std::filesystem::path findConfig(std::filesystem::path file);

/// Paths for where to put fifos that handles things that is local to the
/// current process. For example ttys for consoles or lsp-paths
std::filesystem::path standardLocalFifoDirectory();
std::filesystem::path standardProcessFifoDirectory();

/// A file that other processes can print to to write to the log
std::filesystem::path standardConsoleTtyPipePath();

/// Same as standardConsoleTtyPipePath but for error messages
std::filesystem::path standardErrorTtyPipePath();

void cleanUpOldLocalPipes();

/// Fifo paths for the fifo server/server
std::filesystem::path fifoClientInPath();
std::filesystem::path fifoClientOutPath();
