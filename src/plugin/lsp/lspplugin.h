#pragma once

#include "completion/icompletionsource.h"
#include <filesystem>
#include <memory>
#include <vector>

class LspPluginInstance;

class LspPlugin {
public:
    LspPlugin() = default;
    LspPlugin(const LspPlugin &) = delete;
    LspPlugin(LspPlugin &&) = delete;
    LspPlugin &operator=(const LspPlugin &) = delete;
    LspPlugin &operator=(LspPlugin &&) = delete;

    ~LspPlugin();

    /// Find a appropriate language server for a specific filename
    /// @returns nullptr if no language server exist for the current file type
    LspPluginInstance *findInstance(std::filesystem::path filename);

    std::vector<std::unique_ptr<LspPluginInstance>> _instances;
};

class LspComplete public ICompletionSource{public : };
