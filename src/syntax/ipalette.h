#pragma once

#include "files/filesystem.h"
#include "meditfwd.h"
#include "text/formattype.h"
#include <string>

class IPalette {
public:
    struct BasicPalette {
        FormatType standard, lineNumbers, statement, comment, currentLine,
            string;
    };

    virtual ~IPalette() = default;

    virtual FormatType getFormat(std::string name) const = 0;

    //! Register changes of palette to screen
    virtual void load(filesystem::path) = 0;
    virtual bool update(IScreen &screen) = 0;

    //! Used to get the most basic colors used most often
    virtual const BasicPalette &palette() const = 0;
};
