#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

constexpr auto helpStr = R"_(
usage
code_playback scriptfile.txt [options]

flags:
--help                 print this text
--profiling            enable profiling
)_";

struct CodePlaybackSettings {
    std::filesystem::path scriptFile;
    int viewportWidth = 60;
    int viewportHeight = 15;
    int fontSize = 30;
    bool shouldEnableProfiling = false;

    CodePlaybackSettings(int argc, char **argv) {
        auto args = std::vector<std::string>{argv + 1, argv + argc};

        for (size_t i = 0; i < args.size(); ++i) {
            auto arg = args.at(i);

            if (arg == "--help") {
                std::cout << helpStr << std::endl;
                std::exit(0);
            }
            else if (arg == "--profiling") {
                std::cout << "profiling enabled..." << std::endl;
                shouldEnableProfiling = true;
            }
            else {
                scriptFile = arg;
            }
        }

        parseScriptFile();
    }

    void parseScriptFile() {
        if (scriptFile.empty()) {
            return;
        }

        auto file = std::ifstream{scriptFile};
        for (std::string line; std::getline(file, line);) {
            // Only parse the first lines
            if (line.rfind("// ", 0) != 0) {
                break;
            }
            line = line.substr(3);
            if (auto f = line.find(":"); f != std::string::npos) {
                auto a = line.substr(0, f);
                auto b = line.substr(f + 1);

                if (a == "height") {
                    viewportHeight = std::stoi(b);
                }
                if (a == "width") {
                    viewportWidth = std::stoi(b);
                }
                if (a == "size") {
                    fontSize = std::stoi(b);
                }
            }
        }
    }
};
