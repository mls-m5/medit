#include "registerdefaultplugins.h"
#include "navigation/headernavigation.h"
#include "plugin/clangformat.h"
#include "plugin/jsonformat.h"
#include "syntax/basichighligting.h"
#include "clang/clangannotation.h"
#include "clang/clangcompletion.h"
#include "clang/clangnavigation.h"

void registerDefaultPlugins() {
    ClangFormat::registerPlugin();
    BasicHighlighting::registerPlugin();
    JsonFormat::registerPlugin();
    HeaderNavigation::registerPlugin();

#ifndef __EMSCRIPTEN__
    ClangCompletion::registerPlugin();
    ClangAnnotation::registerPlugin();
    ClangNavigation::registerPlugin();
#endif
}
