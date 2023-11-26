#include "wordstatistics.h"
#include "script/ienvironment.h"
#include "script/interaction.h"
#include "text/buffer.h"
#include "text/cursorrangeops.h"
#include "text/fstring.h"
#include "text/utf8caseconversion.h"
#include "text/words.h"
#include "views/editor.h"
#include <algorithm>
#include <functional>
#include <memory>
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

void beginPhraseStatistics(std::shared_ptr<IEnvironment> env) {
    auto &editor = env->editor();
    auto &buffer = editor.buffer();

    auto map = std::unordered_map<std::string, int>{};

    size_t minSize = 3;

    int lines = 10;

    for (FStringView line : buffer.lines()) {
        if (--lines == 0) {
            break;
        }
        for (size_t begin = 0; begin < line.size(); ++begin) {
            // end + begin because we do not want to do the same substring twice
            for (size_t end = begin + minSize; end + begin <= line.size();
                 ++end) {
                auto substr = line.substr(begin, end);
                ++map[std::string{substr}];
            }
        }
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
            if (it.second <= 1) {
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
    standardCommands.namedCommands["phrase_statistics"] = beginPhraseStatistics;
}
