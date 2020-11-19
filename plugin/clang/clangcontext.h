#pragma once

#include "clangtranslationunit.h"
#include "clangunsavedfiles.h"
#include "files/filesystem.h"
#include "files/ifile.h"
#include "script/ienvironment.h"
#include "tmpfile.h"
#include "views/editor.h"
#include "clang/clangmodel.h"
#include <clang-c/Index.h>
#include <fstream>
#include <optional>
#include <string>

struct ClangContext {
    std::string locationString;
    std::optional<TmpFile> tmpFile;
    std::string tmpPath;
    ClangUnsavedFile unsavedFile;
    ClangTranslationUnit translationUnit;

    ClangUnsavedFile getUnsaved(IEnvironment &env) {

        auto path = filesystem::absolute(env.editor().file()->path());
        auto &buffer = env.editor().buffer();

        locationString = path.string();
        tmpPath = locationString;

        if (buffer.changed()) {
            tmpFile.reset();
            std::ofstream file(tmpPath);
            buffer.text(file);

            locationString = tmpPath;
        }

        auto &project = env.project();

        auto clangFlags = ClangFlags{project};
        return ClangUnsavedFile{buffer.text(), tmpPath};
    }

    ClangTranslationUnit init(IEnvironment &env, ClangModel &model) {
        auto &project = env.project();
        return ClangTranslationUnit{model.index, project, unsavedFile, tmpPath};
    }

    ClangContext(IEnvironment &env, ClangModel &model)
        : unsavedFile(getUnsaved(env)), translationUnit(init(env, model)) {}
};
