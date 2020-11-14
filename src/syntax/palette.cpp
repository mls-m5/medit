#include "syntax/palette.h"
#include "screen/iscreen.h"
#include "syntax/color.h"
#include "json/json.h"
#include <algorithm>
#include <fstream>

struct Palette::Style {
    Color color;
    Color background;
    FormatType f = 0;

    void setProperty(std::string_view name, Color value) {
        if (name == "foreground") {
            color = value;
        }
        else if (name == "background") {
            background = value;
        }
    }
};

Palette::Palette() = default;

Palette::~Palette() = default;

FormatType Palette::getFormat(std::string name) const {
    if (auto f = _styles.find(name); f != _styles.end()) {
        return f->second.f;
    }
    return {};
}

void Palette::load(std::istream &stream) {
    Json json;
    stream >> json;

    _isChanged = true;

    if (auto f = json.find("palette"); f != json.end()) {
        for (auto &color : *f) {
            _palette[color.name] = Color{color.value};
        }
    }
    else {
        return;
    }

    if (auto f = json.find("style"); f != json.end()) {
        for (auto &style : *f) {
            for (auto &property : style) {
                _styles[style.name].setProperty(property.name,
                                                getColor(property.value));
            }
        }
    }
    else {
        return;
    }
}

void Palette::load(filesystem::path path) {
    std::fstream(path) >> *this;
}

Color Palette::getColor(std::string_view name) const {
    if (auto f = std::find_if(_palette.begin(),
                              _palette.end(),
                              [&name](auto &&a) { return a.first == name; });
        f != _palette.end()) {
        return f->second;
    }
    return {};
}

Color Palette::getStyleColor(std::string_view name) const {
    if (auto f = std::find_if(_styles.begin(),
                              _styles.end(),
                              [&name](auto &&a) { return a.first == name; });
        f != _styles.end()) {
        return f->second.color;
    }
    return {};
}

bool Palette::update(IScreen &screen) {
    if (_isChanged) {
        for (auto &style : _styles) {
            style.second.f =
                screen.addStyle(style.second.color, style.second.background);
        }
        _isChanged = false;

        return true;
    }
    return false;
}
