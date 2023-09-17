
#include "completion/icompletionsource.h"
#include "core/coreenvironment.h"
#include "lspconfiguration.h"
#include "navigation/inavigation.h"
#include "script/ienvironment.h"
#include "syntax/ihighlight.h"
#include "syntax/irename.h"
#include "text/buffer.h"
#include <filesystem>
#include <memory>
#include <unordered_map>

namespace lsp {
class LspClient;
}

/// TODO: Make more generic to handle other languages language servers
#warning                                                                       \
    "Create a separate class called LspInstance (rename this class) and make a class that creates a class for each language and calls functions for all"
class LspPluginInstance {
public:
    LspPluginInstance();

    void init();

    LspPluginInstance(const LspPluginInstance &) = delete;
    LspPluginInstance(LspPluginInstance &&) = delete;
    LspPluginInstance &operator=(const LspPluginInstance &) = delete;
    LspPluginInstance &operator=(LspPluginInstance &&) = delete;

    ~LspPluginInstance();

    lsp::LspClient &client() {
        return *_client;
    }

    CoreEnvironment &core() {
        return *_core;
    }

    [[nodiscard]] const LspConfiguration &config() const {
        return _config;
    }

    void updateBuffer(Buffer &);

    static void registerPlugin(CoreEnvironment &core, Plugins &);

private:
    void bufferEvent(BufferEvent &event);

    void handleSemanticsTokens(std::shared_ptr<Buffer> buffer,
                               std::vector<long>);

    void requestSemanticsToken(std::shared_ptr<Buffer> buffer);

    CoreEnvironment *_core = nullptr;

    std::unique_ptr<lsp::LspClient> _client;
    std::unordered_map<std::string, long> _bufferVersions;

    LspConfiguration _config;
};

class LspComplete : public ICompletionSource {
public:
    explicit LspComplete(std::shared_ptr<LspPluginInstance> lsp)
        : _lsp(std::move(lsp)) {}

    void list(std::shared_ptr<IEnvironment>, CompleteCallbackT) override;
    bool shouldComplete(std::shared_ptr<IEnvironment> env) override;
    int priority() override {
        return 100;
    }

    std::shared_ptr<LspPluginInstance> _lsp;
};

class LspNavigation : public INavigation {
public:
    explicit LspNavigation(std::shared_ptr<LspPluginInstance> lsp)
        : _lsp(std::move(lsp)) {}

    /// Goto symbol,
    /// @return true if symbol was found
    bool gotoSymbol(std::shared_ptr<IEnvironment> env) override;

    std::shared_ptr<LspPluginInstance> _lsp;
};

class LspHighlight : public IHighlight {
public:
    explicit LspHighlight(std::shared_ptr<LspPluginInstance> lsp)
        : _lsp(std::move(lsp)) {}

    bool highlight(Buffer &) override;
    int priority() override {
        return 100;
    }

private:
    bool shouldEnable(std::filesystem::path);
    std::shared_ptr<LspPluginInstance> _lsp;
};

class LspRenameInstance : public IRename {
public:
    bool shouldEnable(std::filesystem::path path) const;

    bool doesSupportPrepapre() override;

    bool prepare(std::shared_ptr<IEnvironment> env,
                 std::function<void(PrepareCallbackArgs)>) override;

    /// If the current buffer is a operation buffer
    /// Use the contents to perform a rename
    bool rename(std::shared_ptr<IEnvironment>,
                RenameArgs,
                std::function<void(const Changes &)> callback) override;

    int priority() const override {
        return 100;
    }

    explicit LspRenameInstance(std::shared_ptr<LspPluginInstance> lsp)
        : _lsp(std::move(lsp)) {}

private:
    std::shared_ptr<LspPluginInstance> _lsp;
};
