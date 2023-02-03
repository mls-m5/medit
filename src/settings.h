#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

enum class UiStyle {
    Terminal,
    Matgui,
};

struct Settings {
    std::filesystem::path file;
    UiStyle style = UiStyle::Matgui;

    static constexpr auto helpStr = R"_(
medit text editor

usage:

medit -[flags] [file]

flags:

--gui                 force start with graphic interface
--cli                 start in terminal
--help                print this string

    )_";

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
                style = UiStyle::Terminal;
            }
            else if (arg == "--help") {
                std::cout << helpStr << std::endl;
                std::exit(0);
            }
            else {
                if (arg.rfind("-") != 0) {
                    file = args.at(i);
                }
            }
        }
    }
};
