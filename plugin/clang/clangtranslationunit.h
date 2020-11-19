#pragma once

#include "clangflags.h"
#include "clangunsavedfiles.h"
#include "files/project.h"
#include "clang-c/Index.h"

struct ClangTranslationUnit {
    CXTranslationUnit translationUnit = 0;
    ClangFlags clangFlags;

    ClangTranslationUnit(CXIndex index,
                         Project &project,
                         ClangUnsavedFile &unsavedFile,
                         filesystem::path path)
        : clangFlags(project) {
        auto locationString = path.string();

        translationUnit = clang_parseTranslationUnit(index,
                                                     locationString.c_str(),
                                                     clangFlags.args.data(),
                                                     clangFlags.args.size(),
                                                     &unsavedFile.clangFile,
                                                     1,
                                                     0);
    }

    ~ClangTranslationUnit() {
        clang_disposeTranslationUnit(translationUnit);
    }
};
