#pragma once

#include "clang-c/Index.h"

//! Data related to clang that is shared between different parts of the program
class ClangModel {
public:
    ClangModel();
    ~ClangModel();

    CXIndex index;
};

//! Return the shared object
ClangModel *getClangModel();
