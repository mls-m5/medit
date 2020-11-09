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
    };

private:
    std::vector<CompleteResult> complete(Cursor cursor,
                                         filesystem::path filePath);

    //! @see ICompletionSource
    bool shouldComplete(IEnvironment &) override;

    //! @ ICompletionSource
    CompletionList list(IEnvironment &env) override;
};
