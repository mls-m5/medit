#include "profiler.h"
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

struct DebugFrame {
    bool start = true;
    std::string_view name;
    std::string_view data2 = {};
    uint_least32_t line = 0;
    int tid = -1;

    std::chrono::high_resolution_clock::time_point ts =
        std::chrono::high_resolution_clock::now();
};

/// Data that does not have a duration
struct InstantData {
    std::string data;
    int tid = -1;

    std::chrono::high_resolution_clock::time_point ts =
        std::chrono::high_resolution_clock::now();
};

bool shouldEnableProfiling = false;

int64_t convertTimePointToMicroseconds(
    std::chrono::high_resolution_clock::time_point tp) {
    auto duration_since_epoch = tp.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(
               duration_since_epoch)
        .count();
}

struct GlobalData {
    GlobalData(const GlobalData &) = delete;
    GlobalData(GlobalData &&) = delete;
    GlobalData &operator=(const GlobalData &) = delete;
    GlobalData &operator=(GlobalData &&) = delete;

    std::vector<struct ThreadData *> threadFrameDatas;

    std::vector<DebugFrame> frames;
    std::vector<InstantData> instants;

    ~GlobalData() {
        /// Collect all data

        if (!shouldEnableProfiling) {
            return;
        }

        auto file = std::ofstream{"medit_profile_log.json"};
        for (auto &frame : frames) {
            file << "collectede frame << " << frame.name << " ts "
                 << convertTimePointToMicroseconds(frame.ts) << "\n";
        }
    }

private:
    GlobalData() = default;
    friend GlobalData &globalData();
};

/// Should not be accessed a lot, could be performance hog due to static
GlobalData &globalData() {
    static auto data = GlobalData{};
    return data;
}

struct ThreadData {
    std::vector<DebugFrame> frames;
    std::vector<InstantData> instants;
    int id = generateProfilingThreadId();

    ThreadData() {
        auto &global = globalData();
        global.threadFrameDatas.push_back(this);
    }

    ~ThreadData() {
        auto &global = globalData();
        global.frames.insert(
            global.frames.begin(), frames.begin(), frames.end());
        frames.clear();
        global.instants.insert(
            global.instants.begin(), instants.begin(), instants.end());
    }

    void enterFrame(std::string_view name,
                    std::string_view data2,
                    uint_least32_t line) {
        frames.push_back({
            .start = true,
            .name = name,
            .data2 = data2,
            .line = line,
            .tid = id,
        });
    }

    void exitFrame(std::string_view name,
                   std::string_view data2,
                   uint_least32_t line) {
        frames.push_back({
            .start = false,
            .name = name,
            .data2 = data2,
            .line = line,
            .tid = id,
        });
    }

    void instant(std::string_view data) {
        instants.push_back({
            .data = std::string{data},
            .tid = id,
        });
    }

    /// Just generate a id that starts with 1 and continue upwards, for
    /// readability
    int generateProfilingThreadId() {
        static auto id = int{0};
        return ++id;
    }
};

thread_local ThreadData localProfilingThreadData;

} // namespace

ProfileDuration::ProfileDuration(std::string_view name,
                                 std::string_view data2,
                                 int line)
    : name{name}
    , data2{data2}
    , line{line} {
    if (!shouldEnableProfiling) {
        return;
    }

    localProfilingThreadData.enterFrame(name, data2, line);
}

ProfileDuration::~ProfileDuration() {
    if (!shouldEnableProfiling) {
        return;
    }

    localProfilingThreadData.exitFrame(name, data2, line);
}

void enableProfiling() {
    shouldEnableProfiling = true;
}

void profileInstant(std::string_view value) {
    localProfilingThreadData.instant(value);
}
