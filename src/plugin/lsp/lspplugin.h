
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
class LspPlugin {
public:
    LspPlugin();

    void init();

    LspPlugin(const LspPlugin &) = delete;
    LspPlugin(LspPlugin &&) = delete;
    LspPlugin &operator=(const LspPlugin &) = delete;
    LspPlugin &operator=(LspPlugin &&) = delete;

    ~LspPlugin();

    void bufferEvent(BufferEvent &event);

    static void registerPlugin(CoreEnvironment &core, Plugins &);

    void handleSemanticsTokens(std::shared_ptr<Buffer> buffer,
                               std::vector<long>);

    void requestSemanticsToken(std::shared_ptr<Buffer> buffer);

    //    /// TODO: Should probably be owned by the sub-plugins via shared_ptrs
    //    static LspPlugin &instance() {
    //        static LspPlugin plugin;
    //        return plugin;
    //    }

    lsp::LspClient &client() {
        return *_client;
    }

    void updateBuffer(Buffer &);

    CoreEnvironment &core() {
        return *_core;
    }

    [[nodiscard]] const LspConfiguration &config() const {
        return _config;
    }

private:
    CoreEnvironment *_core = nullptr;

    std::unique_ptr<lsp::LspClient> _client;
    std::unordered_map<std::string, long> _bufferVersions;

    LspConfiguration _config;
};

class LspComplete : public ICompletionSource {
public:
    explicit LspComplete(std::shared_ptr<LspPlugin> lsp)
        : _lsp(std::move(lsp)) {}

    void list(std::shared_ptr<IEnvironment>, CompleteCallbackT) override;
    bool shouldComplete(std::shared_ptr<IEnvironment> env) override;
    int priority() override {
        return 100;
    }

    std::shared_ptr<LspPlugin> _lsp;
};

class LspNavigation : public INavigation {
public:
    explicit LspNavigation(std::shared_ptr<LspPlugin> lsp)
        : _lsp(std::move(lsp)) {}

    /// Goto symbol,
    /// @return true if symbol was found
    bool gotoSymbol(std::shared_ptr<IEnvironment> env) override;

    std::shared_ptr<LspPlugin> _lsp;
};

class LspHighlight : public IHighlight {
public:
    explicit LspHighlight(std::shared_ptr<LspPlugin> lsp)
        : _lsp(std::move(lsp)) {}

    bool shouldEnable(std::filesystem::path) override;
    void highlight(Buffer &) override;
    int priority() override {
        return 100;
    }

private:
    std::shared_ptr<LspPlugin> _lsp;
};

class LspRename : public IRename {
public:
    bool shouldEnable(std::filesystem::path path) const override;

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

    explicit LspRename(std::shared_ptr<LspPlugin> lsp)
        : _lsp(std::move(lsp)) {}

private:
    std::shared_ptr<LspPlugin> _lsp;
};
