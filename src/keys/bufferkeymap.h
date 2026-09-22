#pragma once

#include "script/command.h"
#include "text/fstring.h"
#include "text/fstringview.h"
#include <functional>

class BufferKeyMap {
public:
    using MapType = std::vector<std::pair<FString, CommandT>>;

    enum MatchType {
        NoMatch,
        Match,
        PartialMatch,
    };

    using ReturnT = std::pair<MatchType, CommandT>;
    using MappingFunctionT = std::function<ReturnT(FStringView)>;

    //! @returns second = CommandBlock pointing to matching commandblock if
    //! match else second = nullptr, first = kind of match
    std::pair<MatchType, CommandT> match(FStringView str) {
        auto best = NoMatch;
        for (auto &item : _map) {
            auto m = match(item.first, str);
            if (m == Match) {
                return {Match, item.second};
            }
            else if (m == PartialMatch) {
                best = PartialMatch;
            }
        }

        if (_customMatchFunction) {
            auto match = _customMatchFunction(str);

            if (match.first == Match) {
                return {Match, match.second};
            }
            else if (match.first == PartialMatch) {
                return {PartialMatch, {}};
            }
        }

        return {best, {}};
    }

    BufferKeyMap(MapType map)
        : _map(std::move(map)) {}

    BufferKeyMap() = default;
    BufferKeyMap(const BufferKeyMap &) = default;
    BufferKeyMap &operator=(const BufferKeyMap &) = default;
    BufferKeyMap(BufferKeyMap &&) = default;
    BufferKeyMap &operator=(BufferKeyMap &&) = default;
    ~BufferKeyMap() = default;

    void customMatchFunction(MappingFunctionT f) {
        _customMatchFunction = f;
    }

private:
    static MatchType match(FStringView mapped, FStringView test) {
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
    MappingFunctionT _customMatchFunction;
};
