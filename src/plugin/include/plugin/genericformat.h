#pragma once

#include "syntax/iformat.h"

// Format file with a installed command line tool
class GenericFormat : public IFormat {
public:
    //! @see IFormat
    bool format(Editor &);
};
