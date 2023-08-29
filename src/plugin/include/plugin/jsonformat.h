#pragma once

#include "syntax/iformat.h"

class JsonFormat : public IFormat {
public:
    //! @see IFormat
    bool format(Editor &);
};
