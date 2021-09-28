#include "registerdefaultplugins.h"
#include "plugin/clangformat.h"
#include "plugin/jsonformat.h"
#include "syntax/basichighligting.h"
#include "clang/clangannotation.h"
#include "clang/clangcompletion.h"

void registerDefaultPlugins() {
    ClangFormat::registerPlugin();
    BasicHighlighting::registerPlugin();
    JsonFormat::registerPlugin();

#ifndef __EMSCRIPTEN__
    ClangCompletion::registerPlugin();
    ClangAnnotation::registerPlugin();
#endif
}
