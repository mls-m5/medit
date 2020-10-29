#pragma once

#ifdef USE_EXPERIMENTAL_FILESYSTEME

#include <experimental/filesystem>

namespace filesystem = std::experimental::filesystem;

#else

#include <filesystem>

namespace filesystem = std::filesystem;

#endif
