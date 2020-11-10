#pragma once

#include "files/filesystem.h"

bool isCpp(filesystem::path path) {
    auto extension = path.extension();
    return extension == ".h" || extension == ".cpp";
}
