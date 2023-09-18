
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
class LspPluginInstance {
public:
    LspPluginInstance(CoreEnvironment *core);

    LspPluginInstance(const LspPluginInstance &) = delete;
    LspPluginInstance(LspPluginInstance &&) = delete;
    LspPluginInstance &operator=(const LspPluginInstance &) = delete;
    LspPluginInstance &operator=(LspPluginInstance &&) = delete;

    ~LspPluginInstance();

    [[nodiscard]] lsp::LspClient *client(std::filesystem::path path) {
        if (auto i = instance(path)) {
            return i->_client.get();
        }

        /// Create new client here if it does not exist
        return nullptr;
    }

    [[nodiscard]] CoreEnvironment &core() {
        return *_core;
    }

    [[nodiscard]] const LspConfiguration *config(
        std::filesystem::path path) const {
        if (auto i = instance(path)) {
            return &i->_config;
        }
        return nullptr;
    }

    bool updateBuffer(Buffer &);

    static void registerPlugin(CoreEnvironment &core, Plugins &);

    //        bool isEnabled() {}

    struct Instance {
        Instance(LspConfiguration, LspPluginInstance *parent);

        Instance(Instance &&) = default;
        ~Instance() = default;

        LspConfiguration _config;
        std::unique_ptr<lsp::LspClient> _client;
    };

    [[nodiscard]] Instance *instance(std::filesystem::path path) {
        for (auto &ext : _unsupportedExtensions) {
            if (path.extension() == ext) {
                return nullptr;
            }
        }

        for (auto &instance : _instances) {
            if (instance->_config.isFileSupported(path)) {
                return instance.get();
            }
        }

        return createInstance(path);
        //        return nullptr;
    }

private:
    [[nodiscard]] Instance *createInstance(std::filesystem::path path);

    void bufferEvent(BufferEvent &event);

    void handleSemanticsTokens(std::shared_ptr<Buffer> buffer,
                               std::vector<long>);

    CoreEnvironment *_core = nullptr;

    std::unordered_map<std::string, long> _bufferVersions;

    void requestSemanticsToken(std::shared_ptr<Buffer> buffer,
                               Instance &instance);

    [[nodiscard]] Instance *instance(std::filesystem::path path) const {
        for (auto &instance : _instances) {
            if (instance->_config.isFileSupported(path)) {
                return instance.get();
            }
        }
        return nullptr;
    }

    std::vector<std::unique_ptr<Instance>> _instances;
    std::vector<std::filesystem::path> _unsupportedExtensions;
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
    //    bool shouldEnable(std::filesystem::path);
    std::shared_ptr<LspPluginInstance> _lsp;
};

class LspRenameInstance : public IRename {
public:
    //    bool shouldEnable(std::filesystem::path path) const;

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
