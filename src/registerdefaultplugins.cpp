#include "registerdefaultplugins.h"
#include "navigation/headernavigation.h"
#include "plugin/clangformat.h"
#include "plugin/jsonformat.h"
#include "syntax/basichighligting.h"

#ifdef ENABLE_LEGACY_CLANG_PLUGIN
#include "clang/clangannotation.h"
#include "clang/clangcompletion.h"
#include "clang/clangnavigation.h"
#endif

void registerDefaultPlugins() {
    ClangFormat::registerPlugin();
    BasicHighlighting::registerPlugin();
    JsonFormat::registerPlugin();
    HeaderNavigation::registerPlugin();

#ifdef ENABLE_LEGACY_CLANG_PLUGIN
    ClangCompletion::registerPlugin();
    ClangAnnotation::registerPlugin();
    ClangNavigation::registerPlugin();
#endif
}
