#pragma once

#include "files/filesystem.h"
#include "meditfwd.h"
#include "syntax/ipalette.h"
#include "text/formattype.h"
#include <iosfwd>
#include <map>

class Palette : public IPalette {
    struct Style;

    bool _isChanged = true;

    std::map<std::string, Color> _palette;
    std::map<std::string, Style> _styles;

public:
    Palette();
    ~Palette();

    void load(std::istream &stream);
    void load(filesystem::path);

    friend std::istream &operator>>(std::istream &stream, Palette &palette) {
        palette.load(stream);
        return stream;
    }

    FormatType getFormat(std::string name) const;
    Color getColor(std::string_view name) const;
    Color getStyleColor(std::string_view name) const;

    //! Register changes of palette to screen
    bool update(IScreen &screen);
};
