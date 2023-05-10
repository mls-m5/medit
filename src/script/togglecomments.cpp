#include "togglecomments.h"
#include "ienvironment.h"
#include "text/buffer.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include <cctype>

void toggleComments(std::shared_ptr<IEnvironment> env) {
    auto &e = env->editor();
    auto &buffer = e.buffer();

    auto selection = e.selection();

    auto startLine = selection.begin().y();
    auto stopLine = selection.end().y();

    if (startLine == stopLine) {
        // Does not comment the last line of the file... well well
        stopLine += 1;
    }

    // Also returns true for lines with only spaces to handle logic below
    auto hasComment = [&buffer](size_t i) {
        auto &line = buffer.lineAt(i);
        int slashCount = 0;
        bool hasContent = false;
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
                hasContent = true;
                break;
            }
        }
        return !hasContent;
    };

    // If on line do not have comments, it is interpretted as comments needs to
    // be added on toggle
    bool totalHasComment = true;
    for (size_t i = startLine; i < stopLine; ++i) {
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
                        erase({buffer.cursor({col - 2, i}),
                               buffer.cursor({col + 1, i})});
                    }
                    else {
                        erase({buffer.cursor({col - 2, i}),
                               buffer.cursor({col, i})});
                    }
                }
            }
        }
    }
    else {
        // Add comments
        for (size_t i = startLine; i < stopLine; ++i) {
            auto lineStart = buffer.cursor({0, i});

            bool hasContent = false;
            auto lineEnd = end(lineStart);
            for (auto it = lineStart; it != lineEnd; ++it) {
                if (!std::isspace(content(it))) {
                    hasContent = true;
                    break;
                }
            }
            // Only comment away lines with non space content
            if (hasContent) {
                insert(lineStart, FString{"// "});
            }
        }
    }
}
