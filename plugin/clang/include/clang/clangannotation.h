#pragma once

#include "syntax/iannotation.h"
#include <meditfwd.h>

class ClangAnnotation : public IAnnotation {
public:
    //! @see IAnnotation interface
    bool annotate(IEnvironment &env) override;

    //! @see IAnnotation interface
    bool shouldEnable(filesystem::path) override;
};
