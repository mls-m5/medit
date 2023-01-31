#pragma once

#include "completion/icompletionsource.h"
#include "core/plugins.h"
#include "script/iscope.h"
#include "text/buffer.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include <set>

class WordCompletions : public ICompletionSource {
public:
    // @see ICompletions
    void list(std::shared_ptr<IScope> env,
              CompleteCallbackT callback) override {
        std::set<std::string> words;

        auto &buffer = env->editor().buffer();
        const auto end = buffer.end();

        for (auto cursor = buffer.begin(); cursor != end;
             cursor = wordEnd(cursor)) {

            auto start = wordBegin(cursor);
            cursor = right(cursor);

            if (!isalnum(content(start).at(0))) {
                continue;
            }

            auto range = CursorRange(start, cursor);

            auto content = std::string{::content(range).front()};

            words.insert(content);
        }

        CompletionList ret;
        ret.reserve(words.size());
        for (auto &word : words) {
            ret.push_back({word, ""});
        }
        callback(ret);
    }

    //! @see ICompletionSource
    bool shouldComplete(std::shared_ptr<IScope>) override {
        return true;
    }

    //! @see ICompletionSource
    int priority() override {
        return -1;
    }
};

namespace {

struct Register {
    Register() {
        registerCompletion<WordCompletions>();
    }
} r;

} // namespace
