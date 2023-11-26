#include "wordstatistics.h"
#include "script/interaction.h"
#include "text/buffer.h"
#include "text/cursorrangeops.h"
#include "text/fstring.h"
#include "text/utf8caseconversion.h"
#include "text/words.h"
#include "views/editor.h"
#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

void beginWordStatisticsPlugin(std::shared_ptr<IEnvironment> env) {
    auto &editor = env->editor();

    auto map = std::map<std::string, int>{};

    for (auto word : Words{editor.buffer()}) {
        ++map[toString(word)];
    }

    auto sortedLines = std::vector<FString>{};
    {
        auto lines = std::vector<std::pair<FString, int>>{};

        lines.reserve(map.size());

        for (auto &it : map) {
            if (it.first.empty()) {
                continue;
            }
            if (!isAlpha(it.first.front())) {
                continue;
            }
            lines.push_back({FString{it.first, Palette::identifier} + ": " +
                                 std::to_string(it.second),
                             it.second});
        }

        std::sort(lines.begin(), lines.end(), [](auto &a, auto &b) {
            return a.second > b.second;
        });

        for (auto &line : lines) {
            sortedLines.push_back(std::move(line.first));
        }
    }

    auto interaction = Interaction{
        .text = FString::join(sortedLines),
        .title = "word statistics",
    };

    interaction.begin(env->interactions(), {});
}

void registerWordStatistics(StandardCommands &standardCommands) {
    standardCommands.namedCommands["word_statistics"] =
        beginWordStatisticsPlugin;
}
