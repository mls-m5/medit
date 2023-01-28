#pragma once

#include <filesystem>
#include <string>
#include <vector>

enum class UiStyle {
    Standard,
    Matgui,
};

struct Settings {
    std::filesystem::path file;
    UiStyle style = UiStyle::Matgui;

    Settings(int argc, char **argv) {
        if (argc < 1) {
            return;
        }
        auto args = std::vector<std::string>{argv + 1, argv + argc};

        for (size_t i = 0; i < args.size(); ++i) {
            auto arg = args.at(i);

            if (arg == "--gui") {
                style = UiStyle::Matgui;
            }
            else if (arg == "--cli") {
                style = UiStyle::Standard;
            }
            else {
                if (arg.rfind("-") != 0) {
                    file = args.at(i);
                }
            }
        }
    }
};
