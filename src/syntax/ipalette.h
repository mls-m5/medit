#pragma once

#include "text/formattype.h"
#include <string>

class IPalette {
public:
    virtual ~IPalette() = default;

    virtual FormatType getFormat(std::string name) const = 0;
};
