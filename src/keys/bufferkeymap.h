#pragma once

#include "script/command.h"
#include "text/fstring.h"
#include <optional>

class BufferKeyMap {
public:
    using MapType = std::vector<std::pair<FString, CommandBlock>>;

    enum MatchType {
        NoMatch,
        Match,
        PartialMatch,
    };

    //! @returns second = CommandBlock pointing to matching commandblock if
    //! match else second = nullptr, first = kind of match
    std::pair<MatchType, CommandBlock *> match(const FString &str) {
        auto best = NoMatch;
        for (auto &item : _map) {
            auto m = match(item.first, str);
            if (m == Match) {
                return {Match, &item.second};
            }
            else if (m == PartialMatch) {
                best = PartialMatch;
            }
        }
        return {best, nullptr};
    }

    BufferKeyMap(MapType map) : _map(std::move(map)) {}

    BufferKeyMap() = default;
    BufferKeyMap(const BufferKeyMap &) = default;
    BufferKeyMap &operator=(const BufferKeyMap &) = default;
    BufferKeyMap(BufferKeyMap &&) = default;
    BufferKeyMap &operator=(BufferKeyMap &&) = default;

private:
    static MatchType match(const FString mapped, const FString &test) {
        if (mapped.size() < test.size()) {
            // The typed command is longer than the one tested against
            return NoMatch;
        }

        for (size_t i = 0; i < test.size(); ++i) {
            if (test.at(i).c != mapped.at(i).c) {
                return NoMatch;
            }
        }

        if (test.size() == mapped.size() && test.back().c == mapped.back().c) {
            return Match;
        }

        return PartialMatch;
    }

    MapType _map;
};
