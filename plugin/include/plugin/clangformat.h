#pragma once

#include "syntax/iformat.h"

class ClangFormat : public IFormat {
public:
    //! @see IFormat
    bool format(Editor &);

    static void registerPlugin();
};
