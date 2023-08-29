#pragma once

#include "syntax/iformat.h"

/// TODO: Rename or break apart since it handles html formatting also
class ClangFormat : public IFormat {
public:
    //! @see IFormat
    bool format(Editor &);
};
