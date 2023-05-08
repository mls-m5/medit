#include "registerdefaultplugins.h"
#include "core/plugins.h"
#include "navigation/headernavigation.h"
#include "plugin/clangformat.h"
#include "plugin/jsonformat.h"
#include "plugin/lsp/lspplugin.h"
#include "syntax/basichighligting.h"

#ifdef ENABLE_LEGACY_CLANG_PLUGIN
#include "clang/clangannotation.h"
#include "clang/clangcompletion.h"
#include "clang/clangnavigation.h"
#endif

void registerDefaultPlugins(Plugins &container) {
    container.loadPlugin<ClangFormat>();
    container.loadPlugin<BasicHighlighting>();
    container.loadPlugin<JsonFormat>();
    container.loadPlugin<HeaderNavigation>();

#ifdef ENABLE_LEGACY_CLANG_PLUGIN
    ClangCompletion::registerPlugin();
    ClangAnnotation::registerPlugin();
    ClangNavigation::registerPlugin();
#else
#endif

#ifndef __EMSCRIPTEN__
    LspPlugin::registerPlugin(container);
#endif
}
