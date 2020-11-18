#include "clang/clangcompletion.h"
#include "clangflags.h"
#include "files/ifile.h"
#include "files/project.h"
#include "script/ienvironment.h"
#include "text/buffer.h"
#include "views/editor.h"
#include "clang/clangmodel.h"
#include <clang-c/Index.h>
#include <fstream>

namespace {
struct ClangUnsavedFile {
    ClangUnsavedFile(std::string content, std::string filename)
        : content(std::move(content)), filename(std::move(filename)),
          clangFile({this->content.c_str(),
                     this->filename.c_str(),
                     this->content.size()}) {}
    std::string content;
    std::string filename;
    CXUnsavedFile clangFile;
};

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

} // namespace

std::vector<ClangCompletion::CompleteResult> ClangCompletion::complete(
    IEnvironment &env) {

    auto path = filesystem::absolute(env.editor().file()->path());
    auto cursor = env.editor().cursor();
    auto &buffer = env.editor().buffer();

    auto locationString = path.string();
    std::string tmpPath = locationString;

    if (buffer.changed()) {
        tmpPath = "/tmp/comp_file_aoesutnhaoe.cpp";
        std::ofstream file(tmpPath);
        buffer.text(file);

        locationString = tmpPath;
    }

    auto &project = env.project();

    auto clangFlags = ClangFlags{project};

    auto unsavedFile = ClangUnsavedFile{buffer.text(), tmpPath};

    auto tu =
        ClangTranslationUnit{_model->index, project, unsavedFile, tmpPath};

    auto result = clang_codeCompleteAt(tu.translationUnit,
                                       locationString.c_str(),
                                       cursor.y() + 1,
                                       cursor.x() + 1,
                                       &unsavedFile.clangFile,
                                       1,
                                       clang_defaultCodeCompleteOptions());

    if (!result) {
        //! Todo: Handle this better
        //        throw std::runtime_error("failed completion");

        return {};
    }

    std::vector<CompleteResult> ret;

    for (size_t i = 0; i < result->NumResults; ++i) {
        CXCompletionString completion = result->Results[i].CompletionString;

        // Todo: Iterate over all chunks and handle different types

        CompleteResult listItem;

        for (size_t j = 0; j < clang_getNumCompletionChunks(completion); ++j) {
            auto string = clang_getCompletionChunkText(completion, j);
            auto kind = clang_getCompletionChunkKind(completion, j);

            auto cStr = clang_getCString(string);

            if (kind == CXCompletionChunk_TypedText) {
                listItem.completion += cStr;
            }

            listItem.description += " ";
            listItem.description += cStr;

            clang_disposeString(std::move(string));
        }

        ret.push_back(std::move(listItem));
    }

    clang_disposeCodeCompleteResults(result);

    return ret;
}

ClangCompletion::ClangCompletion() : _model(getClangModel()) {}

ClangCompletion::~ClangCompletion() = default;

bool ClangCompletion::shouldComplete(IEnvironment &env) {
    if (!env.editor().file()) {
        return false;
    }
    auto extension = env.editor().file()->path().extension();
    return extension == ".h" || extension == ".cpp";
}

ICompletionSource::CompletionList ClangCompletion::list(IEnvironment &env) {
    auto list = complete(env);

    ICompletionSource::CompletionList ret;

    for (auto l : list) {
        ret.push_back({l.completion, l.description});
    }

    return ret;
}
