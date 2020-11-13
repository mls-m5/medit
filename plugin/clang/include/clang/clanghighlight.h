#pragma once

#include "meditfwd.h"
#include "syntax/ihighlight.h"

// void clangAnnotate(Editor &);

class ClangHighlight : public IHighlight {
public:
    //! @see IHighlight interface
    bool shouldEnable(filesystem::path path);
    void highlight(Editor &buffer);
};
