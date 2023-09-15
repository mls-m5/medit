#include "registerdefaultplugins.h"
#include "core/coreenvironment.h"
#include "core/plugins.h"
#include "navigation/headernavigation.h"
#include "plugin/gdbdebugger.h"
#include "plugin/genericformat.h"
#include "plugin/git.h"
#include "plugin/jsonformat.h"
#include "plugin/lsp/lspplugin.h"
#include "script/standardcommands.h"
#include "syntax/basichighligting.h"
#include "syntax/markdownhighlighting.h"
#include "syntax/markdownnavigation.h"

#ifdef ENABLE_LEGACY_CLANG_PLUGIN
#include "clang/clangannotation.h"
#include "clang/clangcompletion.h"
#include "clang/clangnavigation.h"
#endif

void registerDefaultPlugins(CoreEnvironment &core) {
    auto &container = core.plugins();
    container.loadPlugin<GenericFormat>();
    container.loadPlugin<BasicHighlighting>();
    container.loadPlugin<MarkdownHighlighting>();
    container.loadPlugin<JsonFormat>();
    container.loadPlugin<HeaderNavigation>();
    container.loadPlugin<MarkdownNavigation>();

#ifdef ENABLE_LEGACY_CLANG_PLUGIN
    ClangCompletion::registerPlugin();
    ClangAnnotation::registerPlugin();
    ClangNavigation::registerPlugin();
#else
#endif

#ifndef __EMSCRIPTEN__
    LspPlugin::registerPlugin(core, container);
    container.loadPlugin<GdbDebugger>();
#endif

    registerGitCommands(StandardCommands::get());
}
