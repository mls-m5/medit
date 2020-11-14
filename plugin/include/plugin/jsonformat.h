#pragma once

#include "syntax/IFormat.h"

class JsonFormat : public IFormat {
public:
    //! @see IFormat
    bool format(Editor &);
};
