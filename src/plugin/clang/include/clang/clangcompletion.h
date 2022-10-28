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
    bool shouldComplete(std::shared_ptr<IScope>) override;

    //! @ ICompletionSource
    CompletionList list(std::shared_ptr<IScope> env) override;

    //! @see ICompletionSource
    int priority() override {
        return 10;
    }

    static void registerPlugin();

private:
    ClangModel *_model;
    std::vector<CompleteResult> complete(std::shared_ptr<IScope> env);
};
