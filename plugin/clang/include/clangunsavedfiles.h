#pragma once

#include <clang-c/Index.h>
#include <string>

struct ClangUnsavedFile {
    ClangUnsavedFile(std::string content, std::string filename)
        : content(std::move(content)), filename(std::move(filename)),
          clangFile({this->content.c_str(),
                     this->filename.c_str(),
                     this->content.size()}) {}
    std::string content;
    std::string filename;
    CXUnsavedFile clangFile;
};
