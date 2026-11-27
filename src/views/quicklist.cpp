#include "quicklist.h"
#include "script/ienvironment.h"
#include "syntax/palette.h"
#include "text/buffer.h"
#include <algorithm>
#include <filesystem>
#include <string>

namespace {

constexpr size_t hintGap = 4;
constexpr size_t minWidth = 40;

/// The text with the matched characters highlighted.
FString highlighted(FString text, const std::vector<size_t> &positions) {
    for (auto position : positions) {
        if (position < text.size()) {
            text.at(position).f = Palette::statement;
        }
    }
    return text;
}

bool isBlankPattern(const FString &pattern) {
    return std::all_of(pattern.begin(), pattern.end(), [](const FChar &c) {
        return c.c == ' ' || c.c == '_';
    });
}

} // namespace

QuickList::QuickList(IView *parent, PopulateFunctionT populateFunction)
    : Editor{parent, std::make_shared<Buffer>(true)}
    , _list{parent}
    , _populate{populateFunction} {
    _list.x(0);
    _list.y(1);
    _list.width(20);
    _list.height(20);
}

bool QuickList::keyPress(std::shared_ptr<IEnvironment> env) {
    if (_list.keyPress(env)) {
        return true;
    }
    else {
        auto ret = Editor::keyPress(env);

        filter();
        return ret;
    }
}

QuickList::~QuickList() = default;

void QuickList::draw(IScreen &screen) {
    if (Editor::visible()) {
        Editor::draw(screen);
        _list.draw(screen);
    }
}

void QuickList::callback(std::function<void(std::filesystem::path)> f) {
    _list.callback([f, this](auto &&, size_t, auto &&path) {
        if (path.has_value()) {
            f(std::any_cast<std::string>(path));
        }
        else {
            f({});
        }
        buffer().clear();
    });
}

void QuickList::visible(bool value) {
    _list.visible(value);
    Editor::visible(value);

    if (value) {
        updateList();
    }
}

bool QuickList::visible() const {
    return _list.visible();
}

void QuickList::updateList() {
    // Read once per opening, not per key press: listing project files can be
    // slow, and the list rarely changes while the user types.
    _items = _populate();
    _candidates.clear();
    _candidates.reserve(_items.size());
    for (auto &item : _items) {
        _candidates.push_back({item.text, item.emphasisStart});
    }
    filter();
}

void QuickList::filter() {
    _list.clear();
    _shownValues.clear();

    const auto pattern = FString{buffer().text()};

    struct Row {
        const QuickListItem *item = nullptr;
        std::vector<size_t> positions;
    };
    auto rows = std::vector<Row>{};
    if (isBlankPattern(pattern)) {
        // Nothing typed: keep the order the list was given in.
        for (size_t i = 0; i < _items.size() && i < maxShown; ++i) {
            rows.push_back({&_items[i], {}});
        }
    }
    else {
        for (auto &ranked : fuzzyRank(pattern, _candidates, maxShown)) {
            rows.push_back(
                {&_items.at(ranked.index), std::move(ranked.match.positions)});
        }
    }

    auto textWidth = size_t{0};
    auto hintWidth = size_t{0};
    for (auto &row : rows) {
        textWidth = std::max(textWidth, row.item->text.size());
        hintWidth = std::max(hintWidth, row.item->hint.size());
    }
    // Every row as wide as the widest, with a margin, so the list is drawn as
    // a box over the editor instead of ragged lines.
    const auto rowWidth = std::max(
        minWidth, textWidth + (hintWidth ? hintGap + hintWidth : 0) + 2);

    for (auto &row : rows) {
        auto line = FString{" ", Palette::standard};
        line += highlighted(row.item->text, row.positions);
        if (!row.item->hint.empty()) {
            const auto hintStart = rowWidth - 1 - row.item->hint.size();
            line += FString{std::string(hintStart - line.size(), ' '),
                            Palette::standard};
            line += FString{row.item->hint, Palette::comment};
        }
        line += FString{std::string(rowWidth - line.size(), ' '),
                        Palette::standard};
        _list.addLine(std::move(line), row.item->value);
        _shownValues.push_back(row.item->value);
    }
}

std::vector<std::string> QuickList::shownValues() const {
    return _shownValues;
}
