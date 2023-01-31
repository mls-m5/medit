
#include "lspplugin.h"
#include "core/plugins.h"
#include "registerdefaultplugins.h"

void LspPlugin::registerPlugin() {
    registerNavigation<LspNavigation>();
    registerHighlighting<LspHighlight>();
    registerCompletion<LspComplete>();
}

ICompletionSource::CompletionList LspComplete::list(std::shared_ptr<IScope>) {
    return {{"hello", "there"}, {"you", "!"}};
}
