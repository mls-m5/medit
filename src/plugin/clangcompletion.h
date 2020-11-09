#pragma once

#include "completion/icompletionsource.h"
#include "files/filesystem.h"
#include "text/cursor.h"
#include <string>
#include <vector>

class ClangCompletion : public ICompletionSource {
public:
    struct CompleteResult {
        std::string completion;
        std::string description;
    };

    ClangCompletion();
    ~ClangCompletion();

    //! @see ICompletionSource
    bool shouldComplete(IEnvironment &) override;

    //! @ ICompletionSource
    CompletionList list(IEnvironment &env) override;

private:
    std::vector<CompleteResult> complete(IEnvironment &env);

    struct ClangData;

    std::unique_ptr<ClangData> _data;
};
