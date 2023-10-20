#pragma once

#include "screen/guiscreen.h"
#include "sdlpp/image.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

struct VideoDump {
    std::vector<std::filesystem::path> paths;
    int videoNum = 0;
    int imgNum = 0;
    std::filesystem::path outputPath;

    GuiScreen *screen = nullptr;

    VideoDump(GuiScreen &screen)
        : screen{&screen} {}

    std::filesystem::path generateTmpPngPath(int num) {
        return "/tmp/playback-img-dump-" + std::to_string(num) + ".png";
    }

    void dump() {
        auto surface = screen->readPixels();
        auto path = generateTmpPngPath(imgNum);
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
        img::savePng(surface, path.c_str());
        paths.push_back(path);
        ++imgNum;
    }

    void saveLastScreenshot(std::filesystem::path path,
                            std::filesystem::path to) {
        {
            std::filesystem::copy(
                path, to, std::filesystem::copy_options::overwrite_existing);
        }
    }

    // Finish this segment and create a video and a ending frame
    int finish() {
        if (paths.empty()) {
            return 0;
        }

        auto listFile =
            std::filesystem::path{"/tmp/code_playback_frame-list.txt"};

        std::filesystem::remove_all(listFile);

        auto currentOutputPath =
            outputPath.parent_path() /
            (outputPath.stem().string() + "-" + std::to_string(videoNum) +
             outputPath.extension().string());

        {

            auto file = std::ofstream{listFile};

            auto putFile = [&](auto path) {
                file << "file '" << path.string() << "'\n";
            };
            for (auto &path : paths) {
                putFile(path);
            }

            for (int i = 0; i < 48; ++i) {
                putFile(paths.back());
            }
        }

        std::filesystem::remove_all(currentOutputPath);

        auto command = "ffmpeg -f concat -safe 0 -r 24 -i " +
                       listFile.string() + " -c:v libx264 -pix_fmt yuv420p " +
                       currentOutputPath.string();
        std::cout << command << std::endl;
        auto returnCode = std::system(command.c_str());

        saveLastScreenshot(
            generateTmpPngPath(imgNum - 1), // Why 2? idk... Should be 1
            (currentOutputPath.parent_path() / currentOutputPath.stem())
                .replace_extension(".png"));

        for (auto &path : paths) {
            std::filesystem::remove(path);
        }

        // Cleanup
        paths.clear();

        ++videoNum;

        return returnCode;
    }
};
