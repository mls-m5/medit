
#include "completion/icompletionsource.h"
#include "navigation/inavigation.h"
#include "script/ienvironment.h"
#include "syntax/ihighlight.h"
#include "text/buffer.h"

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

private:
    std::unique_ptr<lsp::LspClient> _client;
};

class LspComplete : public ICompletionSource {
public:
    void list(std::shared_ptr<IScope>, CompleteCallbackT) override;
    bool shouldComplete(std::shared_ptr<IScope>) override {
        LspPlugin::instance();
        return true;
    }
    int priority() override {
        return 100;
    }
};

class LspNavigation : public INavigation {
public:
    bool gotoSymbol(std::shared_ptr<IScope> env) override {
        return false;
    }
};

class LspHighlight : public IHighlight {
public:
    bool shouldEnable(filesystem::path) override {
        LspPlugin::instance();
        return true;
    }
    void highlight(std::shared_ptr<IScope> env) override {}
    void update(const IPalette &palette) override {
        LspPlugin::instance();
    }
    int priority() override {
        return 100;
    }
};
