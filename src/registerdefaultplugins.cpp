#include "registerdefaultplugins.h"
#include "core/coreenvironment.h"
#include "core/plugins.h"
#include "navigation/headernavigation.h"
#include "plugin/gdbdebugger.h"
#include "plugin/genericformat.h"
#include "plugin/git.h"
#include "plugin/jsonformat.h"
#include "plugin/lsp/lspplugininstance.h"
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

#ifndef __EMSCRIPTEN__
    //    LspPluginInstance::registerPlugin(core, container);
    container.loadPlugin<GdbDebugger>();
#endif

    // TODO: Move standardcommands to live in core environment
    registerGitCommands(StandardCommands::get());
}
