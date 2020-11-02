#pragma once

#ifdef USE_EXPERIMENTAL_FILESYSTEM

#include <experimental/filesystem>

namespace filesystem = std::experimental::filesystem;

#else

#include <filesystem>

namespace filesystem = std::filesystem;

#endif
