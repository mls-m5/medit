
#include "views/listview.h"
#include "screen/iscreen.h"
#include "script/ienvironment.h"

struct ListView::ListItem {
    FString text;
    std::any content;
};

ListView::ListView() = default;

ListView::~ListView() = default;

void ListView::addLine(FString text, std::any dataContent) {
    _lines.push_back({std::move(text), std::move(dataContent)});
}

void ListView::clear() {
    _lines.clear();
    _current = 0;
}

void ListView::draw(IScreen &screen) {
    if (!visible()) {
        return;
    }
    size_t ty = 0;

    FString fillStr{std::string(width(), ' '), 0};

    for (const auto &l : _lines) {
        if (ty == height()) {
            break;
        }
        screen.draw(x(), y() + ty, fillStr);
        screen.draw(x(), y() + ty, l.text);
        ++ty;
    }
}

bool ListView::keyPress(IEnvironment &env) {
    switch (env.key().key) {
    case Key::Up:
        if (_current > 0) {
            --_current;
        }
        return true;
    case Key::Down:
        if (_current < _lines.size() - 1) {
            ++_current;
        }
        return true;
    case Key::Text:
        if (env.key().symbol == '\n') {
            if (!_lines.empty()) {
                auto &line = _lines.at(_current);
                _callback(line.text, line.content);
            }
        }
    default:
        return false;
    }
}

void ListView::updateCursor(IScreen &screen) const {
    screen.cursor(x(), y() - yScroll() + _current);
}
