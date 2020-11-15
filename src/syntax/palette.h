#pragma once

#include "files/filesystem.h"
#include "meditfwd.h"
#include "syntax/ipalette.h"
#include "text/formattype.h"
#include <iosfwd>
#include <map>

inline const std::string standardFormatName = "text";

class Palette : public IPalette {
    struct Style;

    BasicPalette _basicPalette;

    bool _isChanged = true;

    std::map<std::string, Color> _palette;
    std::map<std::string, Style> _styles;

public:
    Palette();
    ~Palette();

    friend std::istream &operator>>(std::istream &stream, Palette &palette) {
        palette.load(stream);
        return stream;
    }

    FormatType getFormat(std::string name) const override;
    Color getColor(std::string_view name) const;
    Color getStyleColor(std::string_view name) const;

    //! @see IPalette
    bool update(IScreen &screen) override;
    //! @see IPalette
    void load(filesystem::path) override;
    //! @see IPalette
    void load(std::istream &stream);
    //! @see IPalette
    const BasicPalette &palette() const override {
        return _basicPalette;
    }
};
