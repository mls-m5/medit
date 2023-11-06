#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

constexpr auto helpStr = R"_(
usage
code_playback scriptfile.txt [options]

flags:
--help                 print this text
--out -o               set output directory
--start                start frame number
--start                start frame number
--stop                 stop frame number
--size -s              set font size
--width -x             character width
--height -y            character height
--fps                  frames per second in output
--profiling            enable profiling
--lines-off            disable linenumbers
--cursor               cursor style, block, beam or hidden, standard: beam
)_";

struct CodePlaybackSettings {
    std::filesystem::path scriptFile;
    std::filesystem::path outputPath;
    int viewportWidth = 60;
    int viewportHeight = 15;
    int fontSize = 30;
    bool shouldEnableProfiling = false;

    int startFrameNumber = 0;
    int stopFrameNumber = std::numeric_limits<int>::max();
    int fps = 24;
    bool hasLineNumbers = true;
    std::string cursor = "beam";

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
            else if (arg == "--start") {
                startFrameNumber = std::stoi(args.at(++i));
            }
            else if (arg == "--stop") {
                stopFrameNumber = std::stoi(args.at(++i));
            }
            else if (arg == "--size" || arg == "-s") {
                fontSize = std::stoi(args.at(++i));
            }
            else if (arg == "--width" || arg == "-x") {
                viewportWidth = std::stoi(args.at(++i));
            }
            else if (arg == "--height" || arg == "-y") {
                viewportHeight = std::stoi(args.at(++i));
            }
            else if (arg == "--out" || arg == "-o") {
                outputPath = args.at(++i);
            }
            else if (arg == "--fps") {
                fps = std::stoi(args.at(++i));
            }
            else if (arg == "--lines-off") {
                hasLineNumbers = false;
            }
            else if (arg == "--cursor") {
                cursor = args.at(++i);
            }
            else {
                if (arg.starts_with('-')) {
                    std::cerr << helpStr << "\n";
                    std::cerr << "unknown argument " << arg << "\n";
                    std::exit(1);
                }
                scriptFile = arg;
            }
        }

        if (outputPath.empty()) {
            if (scriptFile.has_parent_path()) {
                outputPath = scriptFile.parent_path() /
                             (scriptFile.stem().string() + "-out" +
                              scriptFile.extension().string());
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
            if (auto f = line.find(": "); f != std::string::npos) {
                auto a = line.substr(0, f);
                auto b = line.substr(f + 2);

                if (a == "height") {
                    viewportHeight = std::stoi(b);
                }
                else if (a == "width") {
                    viewportWidth = std::stoi(b);
                }
                else if (a == "size") {
                    fontSize = std::stoi(b);
                }
                else if (a == "cursor") {
                    cursor = b;
                }
                else if (a == "fps") {
                    fps = std::stoi(b);
                }
                else if (a == "lines") {
                    hasLineNumbers = b != "off";
                }
            }
        }
    }
};
