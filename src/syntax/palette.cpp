#include "syntax/palette.h"
#include "screen/iscreen.h"
#include "syntax/color.h"
#include "json/json.h"
#include <algorithm>
#include <fstream>

struct Palette::Style {
    Color color = {255, 255, 255, true};
    Color background;
    //    bool noBackground = true;
    //    bool noForeground = true;
    FormatType f = 0;

    void setProperty(std::string_view name, Color value) {
        if (name == "foreground") {
            color = value;
        }
        else if (name == "background") {
            background = value;
            //            noBackground = false;
        }
    }

    void setStandard(Color fg, Color bg) {
        if (!color) {
            color = fg;
        }
        if (!background) {
            background = bg;
            //            noBackground = false;
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

    Style standardStyle;

    if (auto f = json.find("style"); f != json.end()) {
        for (auto &style : *f) {
            auto &s = _styles[style.name];
            for (auto &property : style) {
                s.setProperty(property.name, getColor(property.value));
            }

            if (style.name == standardFormatName) {
                standardStyle = s;
            }
        }
    }
    else {
        return;
    }

    for (auto &style : _styles) {
        style.second.setStandard(standardStyle.color, standardStyle.background);
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

Palette::Style *Palette::getStyle(const std::string &name) {
    if (auto f = _styles.find(name); f != _styles.end()) {
        return &f->second;
    }

    return nullptr;
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

void Palette::setFormat(IScreen &screen, Style *style, size_t index) {
    if (!index) {
        throw std::invalid_argument("index needs to be more than 0");
    }

    if (!style) {
        return;
    }

    screen.addStyle(style->color, style->background, index);
    style->f = index;
}

bool Palette::update(IScreen &screen) {
    if (_isChanged) {
        _isChanged = false;

        auto fixFormat = [&](const std::string &name, size_t index) {
            if (auto style = getStyle(name)) {
                setFormat(screen, style, index);
            }
        };

        fixFormat("text", BasicPalette::standard);
        fixFormat("current-line", BasicPalette::currentLine);
        fixFormat("line-numbers", BasicPalette::lineNumbers);
        fixFormat("def:identifier", BasicPalette::identifier);
        fixFormat("def:statement", BasicPalette::statement);
        fixFormat("def:comment", BasicPalette::comment);
        fixFormat("def:string", BasicPalette::string);
        fixFormat("def:type", BasicPalette::type);

        auto standardStyle = _styles[standardFormatName];
        screen.addStyle(standardStyle.color, standardStyle.background, 0);
        screen.addStyle(standardStyle.color, standardStyle.background, 1);

        return true;
    }
    return false;
}
