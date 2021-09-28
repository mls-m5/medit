#pragma once

#include "syntax/iannotation.h"
#include <meditfwd.h>

class ClangAnnotation : public IAnnotation {
public:
    ClangAnnotation();

    //! @see IAnnotation interface
    bool annotate(std::shared_ptr<IEnvironment> env) override;

    //! @see IAnnotation interface
    bool shouldEnable(filesystem::path) override;

    static void registerPlugin();

private:
    struct Data;
    std::unique_ptr<Data> _data;
};
