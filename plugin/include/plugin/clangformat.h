#pragma once

#include "syntax/IFormat.h"

class ClangFormat : public IFormat {
public:
    //! @see IFormat
    bool format(Editor &);
};