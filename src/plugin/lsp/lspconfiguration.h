#pragma once

#include <filesystem>
#include <string>

std::string getLspConfigForLanguage(std::filesystem::path extension);
