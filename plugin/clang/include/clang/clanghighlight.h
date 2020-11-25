#pragma once

#include "meditfwd.h"
#include "syntax/ihighlight.h"
#include "syntax/ipalette.h"

class ClangHighlight : public IHighlight {
public:
    //! @see IHighlight interface
    bool shouldEnable(filesystem::path path);
    void highlight(std::shared_ptr<IEnvironment> env);

    //! @see IHighlight
    void update(const IPalette &palette);

private:
};
