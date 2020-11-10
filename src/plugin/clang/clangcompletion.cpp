#include "clangcompletion.h"
#include "plugin/clang/clangmodel.h"
#include "script/ienvironment.h"
#include "views/editor.h"
#include <clang-c/Index.h>

namespace {

struct UnsavedFile {
    UnsavedFile(std::string content, std::string filename)
        : content(std::move(content)), filename(std::move(filename)),
          clangFile({this->content.c_str(),
                     this->filename.c_str(),
                     this->content.size()}) {}
    std::string content;
    std::string filename;
    CXUnsavedFile clangFile;
};

} // namespace

std::vector<ClangCompletion::CompleteResult> ClangCompletion::complete(
    IEnvironment &env) {

    auto path = filesystem::absolute(env.editor().file()->path());
    auto cursor = env.editor().cursor();
    auto buffer = env.editor().buffer();

    auto locationString = path.string();

    const char *args[2] = {"-std=c++17", "-Iinclude"};

    auto unsavedFile = UnsavedFile{buffer.text(), locationString};

    auto translationUnit = clang_parseTranslationUnit(_model->index,
                                                      locationString.c_str(),
                                                      args,
                                                      2,
                                                      &unsavedFile.clangFile,
                                                      1,
                                                      0);

    auto result = clang_codeCompleteAt(translationUnit,
                                       locationString.c_str(),
                                       cursor.y() + 1,
                                       cursor.x() + 1,
                                       &unsavedFile.clangFile,
                                       1,
                                       clang_defaultCodeCompleteOptions());

    if (!result) {
        //! Todo: Handle this better
        throw std::runtime_error("failed completion");
    }

    std::vector<CompleteResult> ret;

    for (size_t i = 0; i < result->NumResults; ++i) {
        CXCompletionString completion = result->Results[i].CompletionString;

        // Todo: Iterate over all chunks

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
