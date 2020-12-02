#pragma once

#include "completion/icompletionsource.h"
#include "core/plugins.h"
#include "script/ienvironment.h"
#include "text/buffer.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include <set>

class WordCompletions : public ICompletionSource {
public:
    // @see ICompletions
    CompletionList list(std::shared_ptr<IEnvironment> env) override {
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
        return ret;
    }

    //! @see ICompletionSource
    bool shouldComplete(std::shared_ptr<IEnvironment>) override {
        return true;
    }
};

namespace {

struct Register {
    Register() {
        registerCompletion<WordCompletions>();
    }
} r;

} // namespace
