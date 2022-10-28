#include "togglecomments.h"
#include "script/iscope.h"
#include "text/buffer.h"
#include "views/editor.h"

void toggleComments(std::shared_ptr<IScope> env) {
    auto &e = env->editor();
    auto &buffer = e.buffer();

    auto selection = e.selection();

    auto startLine = selection.begin().y();
    auto stopLine = selection.end().y();

    auto hasComment = [&buffer](size_t i) {
        auto &line = buffer.lineAtConst(i);
        int slashCount = 0;
        for (auto fc : line) {
            auto c = fc.c.at(0);
            if (isspace(c)) {
                continue;
            }
            else if (c == '/') {
                ++slashCount;
                if (slashCount == 2) {
                    return true;
                }
            }
            else {
                break;
            }
        }
        return false;
    };

    // If on line do not have comments, it is interpretted as comments needs to
    // be added on toggle
    bool totalHasComment = true;
    for (size_t i = startLine; i <= stopLine; ++i) {
        if (!hasComment(i)) {
            totalHasComment = false;
            break;
        }
    }

    if (totalHasComment) {
        // Remove one layer of comments
        for (size_t i = startLine; i <= stopLine; ++i) {
            auto &line = buffer.lineAt(i);
            for (size_t col = 2; col < line.size(); ++col) {
                if (line.at(col - 2).c == '/' && line.at(col - 1).c == '/') {
                    if (line.at(col).c == ' ') {
                        line.erase(col - 2, 3); // Also remove a space character
                    }
                    else {
                        line.erase(col - 2, 2);
                    }
                }
            }
        }
    }
    else {
        // Add comments
        for (size_t i = startLine; i <= stopLine; ++i) {
            auto &line = buffer.lineAt(i);
            line.insert(line.begin(), FString{"// "});
        }
    }
}
