#include "clangmodel.h"
#include "clang-c/Index.h"

ClangModel *getClangModel() {
    static ClangModel model;
    return &model;
}

ClangModel::ClangModel() : index(clang_createIndex(0, 0)) {}

ClangModel::~ClangModel() {
    clang_disposeIndex(index);
}
