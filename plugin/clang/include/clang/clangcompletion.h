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
    bool shouldComplete(std::shared_ptr<IEnvironment> ) override;

    //! @ ICompletionSource
    CompletionList list(std::shared_ptr<IEnvironment> env) override;

private:
    ClangModel *_model;
    std::vector<CompleteResult> complete(std::shared_ptr<IEnvironment> env);
};
