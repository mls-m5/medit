#include "clangcompletion.h"
#include "script/ienvironment.h"
#include "views/editor.h"
#include <clang-c/Index.h>

std::vector<ClangCompletion::CompleteResult> ClangCompletion::complete(
    Cursor cursor, filesystem::path path) {
    auto locationString = path.string();
    auto result = clang_codeCompleteAt(nullptr,
                                       locationString.c_str(),
                                       cursor.y() + 1,
                                       cursor.x() + 1,
                                       nullptr,
                                       0,
                                       clang_defaultCodeCompleteOptions());

    if (!result) {
        //! Todo: Handle this better
        throw std::runtime_error("failed completion");
    }

    std::vector<CompleteResult> ret;

    for (size_t i = 0; i < result->NumResults; ++i) {
        CXCompletionString completion = result->Results[i].CompletionString;

        // Todo: Iterate over all chunks
        auto string = clang_getCompletionChunkText(completion, 0);

        auto cStr = clang_getCString(string);

        ret.push_back({cStr});

        clang_disposeString(string);
    }

    clang_disposeCodeCompleteResults(result);

    return ret;
}

bool ClangCompletion::shouldComplete(IEnvironment &env) {
    if (!env.editor().file()) {
        return false;
    }
    auto extension = env.editor().file()->path().extension();
    return extension == ".h" || extension == ".cpp";
}

ICompletionSource::CompletionList ClangCompletion::list(IEnvironment &env) {
    auto list = complete(env.editor().cursor(), env.editor().file()->path());

    ICompletionSource::CompletionList ret;

    for (auto l : list) {
        ret.push_back({l.completion});
    }

    return ret;
}
