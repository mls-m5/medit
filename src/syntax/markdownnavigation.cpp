#include "markdownnavigation.h"
#include "files/extensions.h"
#include "script/ienvironment.h"
#include "text/fstring.h"
#include "views/editor.h"

namespace {}

bool MarkdownNavigation::gotoSymbol(std::shared_ptr<IEnvironment> env) {
    auto &e = env->editor();
    auto path = e.path();
    if (!isMarkdown(path)) {
        return false;
    }

    auto cursor = e.cursor();

    auto &lines = e.buffer().lines();
    auto &line = lines.at(cursor.y());

    size_t leftBracket = FString::npos;

    for (int i = cursor.x(); i > 0; --i) {
        auto c = line.at(i);

        if (c == '[') {
            auto j = i - 1;
            if (j >= 0 && line.at(j) == '[') {
                leftBracket = i + 1;
            }
        }
    }

    if (leftBracket == FString::npos) {
        return false;
    }

    size_t rightBracket = FString::npos;

    for (int i = cursor.x() + 1; i < line.size(); ++i) {
        auto c = line.at(i);

        if (c == ']') {
            auto j = i + 1;
            if (j < line.size() && line.at(j) == ']') {
                rightBracket = i;
            }
        }
    }

    if (rightBracket == FString::npos) {
        return false;
    }

    // TODO: Implement navigation jump to file without path and without
    // extension
    env->statusMessage(line.substr(leftBracket, rightBracket - leftBracket));

    return false;
}
