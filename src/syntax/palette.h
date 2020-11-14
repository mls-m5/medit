#pragma once

#include "files/filesystem.h"
#include "meditfwd.h"
#include "text/formattype.h"
#include <iosfwd>
#include <map>

class Palette {
    struct Style;

    bool _isChanged = true;

public:
    Palette();
    ~Palette();
    std::map<std::string, Color> _palette;
    std::map<std::string, Style> _styles;

    FormatType getFormat(std::string name) const;

    void load(std::istream &stream);
    void load(filesystem::path);

    friend std::istream &operator>>(std::istream &stream, Palette &palette) {
        palette.load(stream);
        return stream;
    }

    Color getColor(std::string_view name) const;
    Color getStyleColor(std::string_view name) const;

    //! Register changes of palette to screen
    bool update(IScreen &screen);
};
