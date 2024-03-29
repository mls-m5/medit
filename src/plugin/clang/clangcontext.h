#pragma once

#include "clangtranslationunit.h"
#include "clangunsavedfiles.h"
#include "files/filesystem.h"
#include "files/ifile.h"
#include "script/iscope.h"
#include "tmpfile.h"
#include "views/editor.h"
#include "clang/clangmodel.h"
#include <clang-c/Index.h>
#include <fstream>
#include <optional>
#include <string>

struct ClangContext {
    //    std::string locationString;
    std::optional<TmpFile> tmpFile;
    std::string tmpPath;
    //    ClangUnsavedFile unsavedFile;
    ClangTranslationUnit translationUnit;

    //    ClangUnsavedFile getUnsaved(std::shared_ptr<IEnvironment> env) {
    //        auto path = filesystem::absolute(env->editor().file()->path());
    //        auto &buffer = env->editor().buffer();

    //        locationString = path.string();
    //        tmpPath = locationString;

    //        if (buffer.isChanged()) {
    //            tmpFile.emplace(".cpp");
    //            tmpPath = tmpFile->path.string();
    //            //            std::ofstream file(tmpPath);
    //            //            buffer.text(file);
    //            std::ofstream(tmpPath) << buffer;
    //        }

    //        auto &project = env->project();

    //        auto clangFlags = ClangFlags{project};
    //        return ClangUnsavedFile{buffer.text(), locationString, tmpPath};
    //    }

    ClangTranslationUnit init(std::shared_ptr<IScope> env,
                              ClangModel &model) {
        auto &project = env->project();

        auto path = filesystem::absolute(env->editor().file()->path());
        auto &buffer = env->editor().buffer();

        //        locationString = path.string();
        //        tmpPath = locationString;
        tmpPath = path.string();

        //        if (buffer.isChanged()) {
        tmpFile.emplace(".cpp");
        tmpPath = tmpFile->path.string();
        //            std::ofstream file(tmpPath);
        //            buffer.text(file);
        std::ofstream(tmpPath) << buffer;
        //        }

        auto clangFlags = ClangFlags{project};

        return ClangTranslationUnit{
            model.index, project /*, unsavedFile*/, tmpPath};
    }

    ClangContext(std::shared_ptr<IScope> env, ClangModel &model)
        //        : unsavedFile(getUnsaved(env)),
        : translationUnit(init(env, model)) {}

    struct Range {
        Position begin, end;
        CXFile file;
    };

    static Range getRange(CXCursor cursor) {
        Range range;
        auto extent = clang_getCursorExtent(cursor);

        unsigned int line, col;
        auto start = clang_getRangeStart(extent);
        //        clang_getSpellingLocation(start, &range.file, &line, &col,
        //        nullptr);
        clang_getFileLocation(start, &range.file, &line, &col, nullptr);
        range.begin = {col - 1, line - 1};

        auto end = clang_getRangeEnd(extent);
        //        clang_getSpellingLocation(end, nullptr, &line, &col, nullptr);
        clang_getFileLocation(end, nullptr, &line, &col, nullptr);
        range.end = {col - 1, line - 1};

        return range;
    }

    struct ClangLocation {
        CXFile file;
        Position position;
    };

    static ClangLocation getLocation(CXCursor cursor) {
        unsigned int line, col;
        ClangLocation location;
        auto cxLocation = clang_getCursorLocation(cursor);
        clang_getFileLocation(cxLocation, &location.file, &line, &col, nullptr);
        location.position.x(col - 1);
        location.position.y(line - 1);
        return location;
    }

    CXCursor getClangCursor(Cursor cursor) {
        auto file = clang_getFile(translationUnit, tmpPath.c_str());

        auto location = clang_getLocation(
            translationUnit, file, cursor.y() + 1, cursor.x() + 1);
        return clang_getCursor(translationUnit, location);
    }
};
