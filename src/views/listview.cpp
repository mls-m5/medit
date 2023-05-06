
#include "views/listview.h"
#include "keys/event.h"
#include "screen/draw.h"
#include "screen/iscreen.h"
#include "script/ienvironment.h"
#include "syntax/palette.h"
#include "text/position.h"

struct ListView::ListItem {
    FString text;
    std::any content;
};

ListView::~ListView() = default;

ListView::ListView(IView *parent)
    : ScrollView{parent} {}

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

    size_t width = 0;

    for (size_t ty = 0, i = yScroll(); ty < height() && i < _lines.size();
         ++i, ++ty) {
        auto &l = _lines.at(i);
        width = std::max(width, l.text.size());
    }
    this->width(width);

    FString fillStr{std::string(this->width(), ' '), Palette::standard};
    FString selFillStr{std::string(this->width(), ' '), Palette::currentLine};

    for (size_t ty = 0, i = yScroll(); ty < height() && i < _lines.size();
         ++i, ++ty) {
        auto &l = _lines.at(i);
        if (ty == height()) {
            break;
        }
        if (i == _current) {
            ::draw(screen, x(), y() + ty, selFillStr);
            ::draw(
                screen, x(), y() + ty, FString{l.text, Palette::currentLine});
        }
        else {
            ::draw(screen, x(), y() + ty, fillStr);
            ::draw(screen, x(), y() + ty, l.text);
        }
    }
}

bool ListView::keyPress(std::shared_ptr<IEnvironment> env) {
    switch (env->key().key) {
    case Key::Up:
        if (_current > 0) {
            current(_current - 1);
        }
        return true;
    case Key::Down:
        if (_current < _lines.size() - 1) {
            current(_current + 1);
        }
        return true;
    case Key::Escape:
        if (_callback) {
            _callback("", -1, {});
            return true;
        }
        return false;
    case Key::Text:
        if (env->key().symbol == '\n') {
            if (!_lines.empty()) {
                auto &line = _lines.at(_current);
                if (_callback) {
                    _callback(line.text, _current, line.content);
                    return true;
                }
                else {
                    return false;
                }
            }
        }
        break;
    default:
        return false;
    }

    return false;
}

void ListView::updateCursor(IScreen &screen) const {
    screen.cursor(x(), y() - yScroll() + _current);
}

void ListView::fitCursor() {
    fitPosition({0, _current});
}

void ListView::current(size_t value) {
    if (_lines.empty()) {
        _current = 0;
    }
    else if (value < _lines.size()) {
        _current = value;
    }
    else {
        _current = _lines.size() - 1;
    }

    fitCursor();
}
