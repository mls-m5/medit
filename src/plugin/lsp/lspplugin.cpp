
#include "lspplugin.h"
#include "core/plugins.h"
#include "registerdefaultplugins.h"

void LspPlugin::registerPlugin() {
    registerNavigation<LspNavigation>();
    registerHighlighting<LspHighlight>();
    registerCompletion<LspComplete>();
}

void LspComplete::list(std::shared_ptr<IScope>, CompleteCallbackT callback) {
    callback({{"hello", "there"}, {"you", "!"}});
}
