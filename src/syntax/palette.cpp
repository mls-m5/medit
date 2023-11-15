#include "syntax/palette.h"
#include "core/archive.h"
#include "screen/iscreen.h"
#include "syntax/color.h"
#include "json/json.h"
#include <algorithm>
#include <fstream>
#include <string>
#include <string_view>

Palette::Palette() = default;

Palette::~Palette() = default;

FormatType Palette::format(std::string name) const {
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
                s.setProperty(property.name, color(property.value));
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

void Palette::load(std::filesystem::path path) {
    std::fstream(path) >> *this;
}

Color Palette::color(std::string_view name) const {
    if (auto f = std::find_if(_palette.begin(),
                              _palette.end(),
                              [&name](auto &&a) { return a.first == name; });
        f != _palette.end()) {
        return f->second;
    }
    return {};
}

Style *Palette::getStyle(const std::string &name) {
    if (auto f = _styles.find(name); f != _styles.end()) {
        return &f->second;
    }

    return nullptr;
}

Color Palette::styleColor(std::string_view name) const {
    if (auto f = std::find_if(_styles.begin(),
                              _styles.end(),
                              [&name](auto &&a) { return a.first == name; });
        f != _styles.end()) {
        return f->second.color;
    }
    return {};
}

void Palette::color(std::string_view name, Color color) {
    _palette[std::string{name}] = color;
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
        fixFormat("outside", BasicPalette::outside);
        fixFormat("line-numbers", BasicPalette::lineNumbers);
        fixFormat("selection", BasicPalette::selection);
        fixFormat("def:identifier", BasicPalette::identifier);
        fixFormat("def:statement", BasicPalette::statement);
        fixFormat("def:comment", BasicPalette::comment);
        fixFormat("def:preprocessor", BasicPalette::preprocessor);
        fixFormat("def:constant", BasicPalette::constant);
        fixFormat("def:string", BasicPalette::string);
        fixFormat("def:type", BasicPalette::type);
        fixFormat("def:error", BasicPalette::error);
        fixFormat("def:warning", BasicPalette::warning);
        fixFormat("def:note", BasicPalette::note);

        auto standardStyle = _styles[standardFormatName];
        screen.addStyle(standardStyle.color, standardStyle.background, 0);
        screen.addStyle(standardStyle.color, standardStyle.background, 1);

        return true;
    }
    return false;
}

void Palette::visit(Archive &arch) {
    ARCH_PAIR(_palette);
    ARCH_PAIR(_styles);
}

template <typename U>
void visitInternal(Archive &arch,
                   std::map<std::string, U> &map,
                   std::string_view name) {
    size_t size = map.size();
    if (!arch.beginList(name, size)) {
        return;
    }

    /// Uggly handling
    if (arch.direction == arch.Out) {
        for (auto &it : map) {
            arch.set("", it.first);
            arch.set("", it.second);
        }
    }
    else {
        map.clear();
        size /= 2; // Since there is two elements per map item
        for (size_t i = 0; i < size; ++i) {
            auto name = arch.get<std::string>("");
            map[name] = arch.get<U>("");
        }
    }

    arch.endChild();
}

void visit(Archive &arch, std::map<std::string, Color> &map) {
    visitInternal(arch, map, "colors");
}

void visit(Archive &arch, std::map<std::string, Style> &map) {
    visitInternal(arch, map, "styles");
}
