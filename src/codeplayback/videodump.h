#pragma once

#include "screen/guiscreen.h"
#include "sdlpp/image.hpp"
#include <filesystem>
#include <fstream>
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
        return "/tmp/playback-img-dump-" + std::to_string(imgNum) + ".png";
    }

    void dump() {
        std::system("rm -f /tmp/playback-img-dump-*.png");

        auto surface = screen->readPixels();
        auto path = generateTmpPngPath(imgNum);
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
        img::savePng(surface, path.c_str());
        paths.push_back(path);
        ++imgNum;
    }

    // Finish this segment and create a video and a ending frame
    int finish() {
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

        auto returnCode = std::system(
            ("ffmpeg -f concat -safe 0 -r 24 -i " + listFile.string() +
             " -c:v libx264 -pix_fmt yuv420p " + currentOutputPath.string())
                .c_str());

        {
            auto lastFrame = generateTmpPngPath(--imgNum);
            std::filesystem::copy(
                lastFrame,
                (outputPath.parent_path() / outputPath.stem())
                    .replace_extension(".png"),
                std::filesystem::copy_options::overwrite_existing);
        }

        for (auto &path : paths) {
            std::filesystem::remove(path);
        }

        // Cleanup
        paths.clear();

        return returnCode;
    }
};
