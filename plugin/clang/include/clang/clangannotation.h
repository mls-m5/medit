#pragma once

#include "syntax/iannotation.h"
#include <meditfwd.h>

class ClangAnnotation : public IAnnotation {
public:
    //! @see IAnnotation interface
    bool annotate(std::shared_ptr<IEnvironment> env) override;

    //! @see IAnnotation interface
    bool shouldEnable(filesystem::path) override;
};
