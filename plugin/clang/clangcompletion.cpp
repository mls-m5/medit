#include "clang/clangcompletion.h"
#include "clangcontext.h"
#include "core/plugins.h"
#include "script/ienvironment.h"
#include "text/cursorops.h"
#include "clang/clangmodel.h"
#include <clang-c/Index.h>

std::vector<ClangCompletion::CompleteResult> ClangCompletion::complete(
    std::shared_ptr<IEnvironment> env) {

    auto context = ClangContext{env, *_model};

    auto cursor = autocompleteWordBegin(env->editor().cursor());

    auto result = clang_codeCompleteAt(context.translationUnit,
                                       context.tmpPath.c_str(),
                                       cursor.y() + 1,
                                       cursor.x() + 1,
                                       nullptr,
                                       0,
                                       // &context.unsavedFile.clangFile,
                                       // 1,
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

bool ClangCompletion::shouldComplete(std::shared_ptr<IEnvironment> env) {
    if (!env->editor().file()) {
        return false;
    }
    auto extension = env->editor().file()->path().extension();
    return extension == ".h" || extension == ".cpp";
}

ICompletionSource::CompletionList ClangCompletion::list(
    std::shared_ptr<IEnvironment> env) {
    auto list = complete(env);

    ICompletionSource::CompletionList ret;

    for (auto l : list) {
        ret.push_back({l.completion, l.description});
    }

    return ret;
}

namespace {

struct Register {
    // This will be modified if the plugin is to be loaded dynamaically
    Register() {
        registerCompletion<ClangCompletion>();
    }
} r;

} // namespace
