
#include "completion/icompletionsource.h"
#include "navigation/inavigation.h"
#include "script/ienvironment.h"
#include "syntax/ihighlight.h"
#include "text/buffer.h"
#include <memory>
#include <unordered_map>

namespace lsp {
class LspClient;
}

/// TODO: Make more generic to handle other languages language servers
class LspPlugin {
public:
    LspPlugin();
    ~LspPlugin();

    void bufferEvent(BufferEvent &event);

    static void registerPlugin(Plugins &);

    void handleSemanticsTokens(std::shared_ptr<Buffer> buffer,
                               std::vector<long>);

    void requestSemanticsToken(std::shared_ptr<Buffer> buffer);

    static LspPlugin &instance() {
        static LspPlugin plugin;
        return plugin;
    }

    lsp::LspClient &client() {
        return *_client;
    }

    void updateBuffer(Buffer &);

private:
    std::unique_ptr<lsp::LspClient> _client;
    std::unordered_map<std::string, long> _bufferVersions;

    CoreEnvironment &_core;
};

class LspComplete : public ICompletionSource {
public:
    void list(std::shared_ptr<IEnvironment>, CompleteCallbackT) override;
    bool shouldComplete(std::shared_ptr<IEnvironment> env) override;
    int priority() override {
        return 100;
    }
};

class LspNavigation : public INavigation {
public:
    bool gotoSymbol(std::shared_ptr<IEnvironment> scope) override;
};

class LspHighlight : public IHighlight {
public:
    bool shouldEnable(filesystem::path) override;
    void highlight(std::shared_ptr<IEnvironment> env) override;
    int priority() override {
        return 100;
    }
};

class LspRename {
    bool shouldEnable(std::shared_ptr<IEnvironment> env) const;

    /// If the current buffer is a operation buffer
    /// Use the contents to perform a rename
    void rename(std::shared_ptr<IEnvironment> env);

    /// Check if a rename is somewhat valid
    /// When
    //    bool prepare(std::shared_ptr<IEnvironment> env,
    //                 std::function<void(bool)> callback);
};
