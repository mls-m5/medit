
#include "completion/icompletionsource.h"
#include "lsp/lspclient.h"
#include "navigation/inavigation.h"
#include "script/ienvironment.h"
#include "syntax/ihighlight.h"
#include "text/buffer.h"

class LspPlugin {
public:
    // Notifications
    void didOpen(IEnvironment &env, Buffer &buffer) {}
    void didClose(IEnvironment &env, Buffer &buffer) {}

    static void registerPlugin();

    static LspPlugin &instance() {
        static LspPlugin plugin;
        return plugin;
    }

    lsp::LspClient _client = {""};
};

class LspComplete : public ICompletionSource {
public:
    CompletionList list(std::shared_ptr<IScope>) override;
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
