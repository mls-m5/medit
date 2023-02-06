#pragma once

#include "files/filesystem.h"
#include "meditfwd.h"
#include "nlohmann/json.hpp"
#include "syntax/color.h"
#include "syntax/palette.h"
#include "text/formattype.h"
#include <iosfwd>
#include <map>

inline const std::string standardFormatName = "text";

struct Style {
    Color color = {255, 255, 255, true};
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

    void setStandard(Color fg, Color bg) {
        if (!color) {
            color = fg;
        }
        if (!background) {
            background = bg;
        }
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Style, color, background, f);
};

class Palette /*: public IPalette*/ {
    bool _isChanged = true;

    std::map<std::string, Color> _palette;
    std::map<std::string, Style> _styles;

    void setFormat(IScreen &screen, Style *, size_t index);
    Style *getStyle(const std::string &name);

public:
    Palette();
    ~Palette();

    friend std::istream &operator>>(std::istream &stream, Palette &palette) {
        palette.load(stream);
        return stream;
    }

    FormatType getFormat(std::string name) const /*override*/;
    Color getColor(std::string_view name) const;
    Color getStyleColor(std::string_view name) const;

    bool update(IScreen &screen) /*override*/;
    void load(filesystem::path) /*override*/;
    void load(std::istream &stream);

    bool isChanged() const {
        return _isChanged;
    }

    enum BasicPalette : FormatType {
        standard = 1,

        identifier,
        statement,
        type,

        comment,
        lineNumbers,

        preprocessor,

        currentLine,
        constant,
        string,

        selection,

        error,
        warning,
        note,
    };

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Palette, _palette, _styles)
};
