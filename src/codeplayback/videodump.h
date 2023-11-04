#pragma once

#include "core/profiler.h"
#include "screen/guiscreen.h"
#include "sdlpp/image.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

struct VideoDump {
    std::vector<std::filesystem::path> paths;
    int videoNum = 0;
    int imgNum = 0;
    std::filesystem::path outputPath;
    std::vector<std::thread> threads; // For encoding

    GuiScreen *screen = nullptr;

    VideoDump(const VideoDump &) = delete;
    VideoDump(VideoDump &&) = delete;
    VideoDump &operator=(const VideoDump &) = delete;
    VideoDump &operator=(VideoDump &&) = delete;

    VideoDump(GuiScreen &screen)
        : screen{&screen} {}

    ~VideoDump() {
        for (auto &thread : threads) {
            thread.join();
        }
    }

    std::filesystem::path generateTmpImgPath(int num) {
        return "/tmp/playback-img-dump-" + std::to_string(num) + ".bmp";
    }

    void dump() {
        auto duration = ProfileDuration{};
        auto surface = screen->readPixels();
        auto path = generateTmpImgPath(imgNum);
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
        {
            auto duration = ProfileDuration{"save"};
            surface.saveBMP(path.c_str());
        }
        paths.push_back(path);
        ++imgNum;
    }

    void saveLastScreenshot(std::filesystem::path path,
                            std::filesystem::path to) {
        {
            auto duration = ProfileDuration{};
            std::filesystem::copy(
                path, to, std::filesystem::copy_options::overwrite_existing);
        }
    }

    // Convert images. (run in another thread)
    static void startConversion(std::vector<std::filesystem::path> paths,
                                std::filesystem::path currentOutputPath) {
        auto duration = ProfileDuration{};

        auto listFile = std::filesystem::path{
            "/tmp/" + currentOutputPath.filename().string() + "-listfile.txt"};

        std::filesystem::remove_all(listFile);

        {

            auto file = std::ofstream{listFile};

            auto putFile = [&](auto path) {
                file << "file '" << path.string() << "'\n";
            };
            for (auto &path : paths) {
                putFile(path);
            }

            putFile(paths.back());
        }

        //        auto command = "ffmpeg -f concat -safe 0 -r 24 -i " +
        //                       listFile.string() +
        //                       " -c:v libx264 -pix_fmt yuv420p -preset
        //                       ultrafast " + currentOutputPath.string();

        int fps = 24;

        auto command = "ffmpeg -f concat -safe 0 -r " + std::to_string(fps) +
                       " -i " + listFile.string() + " -c:v libx264rgb -crf " +
                       std::to_string(18) + " -preset ultrafast " +
                       " -pix_fmt yuv420p " + " " + currentOutputPath.string();
        std::cout << command << std::endl;
        auto returnCode = std::system(command.c_str());

        for (auto &path : paths) {
            std::filesystem::remove(path);
        }

        std::filesystem::remove_all(listFile);

        if (returnCode) {
            throw std::runtime_error{"failed to convert stuff"};
        }
    }

    // Finish this segment and create a video and a ending frame
    int finish() {
        auto duration = ProfileDuration{};
        if (paths.empty()) {
            return 0;
        }

        auto currentOutputPath =
            outputPath.parent_path() /
            (outputPath.stem().string() + "-" + std::to_string(videoNum) +
             outputPath.extension().string());

        std::filesystem::remove_all(currentOutputPath);

        saveLastScreenshot(
            generateTmpImgPath(imgNum - 1),
            (currentOutputPath.parent_path() / currentOutputPath.stem())
                .replace_extension(".bmp"));

        threads.emplace_back([paths = this->paths, currentOutputPath] {
            startConversion(paths, currentOutputPath);
        });

        // Cleanup
        paths.clear();

        ++videoNum;

        return 0;
    }
};
