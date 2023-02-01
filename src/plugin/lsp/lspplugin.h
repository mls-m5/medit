
#include "completion/icompletionsource.h"
#include "navigation/inavigation.h"
#include "script/ienvironment.h"
#include "syntax/ihighlight.h"
#include "text/buffer.h"
#include <unordered_map>

namespace lsp {
class LspClient;
}

class LspPlugin {
public:
    LspPlugin();
    ~LspPlugin();

    void bufferEvent(BufferEvent &event);

    static void registerPlugin();

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
    void list(std::shared_ptr<IScope>, CompleteCallbackT) override;
    bool shouldComplete(std::shared_ptr<IScope> env) override;
    int priority() override {
        return 100;
    }
};

class LspNavigation : public INavigation {
public:
    bool gotoSymbol(std::shared_ptr<IScope> scope) override;
};

class LspHighlight : public IHighlight {
public:
    bool shouldEnable(filesystem::path) override;
    void highlight(std::shared_ptr<IScope> env) override;
    void update(const IPalette &palette) override;
    int priority() override {
        return 100;
    }
};