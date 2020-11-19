#pragma once

#include <clang-c/Index.h>
#include <string>

struct ClangUnsavedFile {
    ClangUnsavedFile(std::string content, std::string filename)
        : content(std::move(content)), filename(std::move(filename)),
          clangFile({this->content.c_str(),
                     this->filename.c_str(),
                     this->content.size()}) {}

    void reset(std::string content, std::string filename) {
        this->content = std::move(content);
        this->filename = std::move(filename);
        this->clangFile = {this->content.c_str(),
                           this->filename.c_str(),
                           this->content.size()};
    }

    ClangUnsavedFile() = default;
    ClangUnsavedFile(const ClangUnsavedFile &) = default;
    ClangUnsavedFile &operator=(const ClangUnsavedFile &) = default;
    //    ClangUnsavedFile(ClangUnsavedFile &&) = default;
    //    ClangUnsavedFile&operator=(ClangUnsavedFile &&) = default;

    std::string content;
    std::string filename;
    CXUnsavedFile clangFile;
};
