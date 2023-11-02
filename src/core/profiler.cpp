#include "profiler.h"
#include "core/threadname.h"
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
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

int64_t convertTimePointToMicroseconds(
    std::chrono::high_resolution_clock::time_point tp) {
    auto duration_since_epoch = tp.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(
               duration_since_epoch)
        .count();
}

std::ostream &operator<<(std::ostream &os, const DebugFrame &df) {
    os << "{";
    os << "\"name\": \"" << df.name << ":" << df.data2 << "@" << df.line
       << "\", ";
    os << "\"ph\": \"" << (df.start ? "B" : "E") << "\", ";
    os << "\"ts\": " << convertTimePointToMicroseconds(df.ts) << ", ";
    os << "\"pid\": 1, ";
    os << "\"tid\": " << df.tid;
    os << "}";

    return os;
}

/// Data that does not have a duration
struct InstantData {
    std::string data;
    int tid = -1;

    std::chrono::high_resolution_clock::time_point ts =
        std::chrono::high_resolution_clock::now();
};

bool shouldEnableProfiling = false;

struct ProfiledData {
    /// Just generate a id that starts with 1 and continue upwards, for
    /// readability
    int generateProfilingThreadId() {
        static auto id = int{0};
        return ++id;
    }

    std::vector<DebugFrame> frames;
    std::vector<InstantData> instants;
    int id = generateProfilingThreadId();
    std::string name;
};

struct GlobalData {
    GlobalData(const GlobalData &) = delete;
    GlobalData(GlobalData &&) = delete;
    GlobalData &operator=(const GlobalData &) = delete;
    GlobalData &operator=(GlobalData &&) = delete;

    std::vector<std::shared_ptr<ProfiledData>> threadFrameDatas;

    //    std::vector<DebugFrame> frames;
    //    std::vector<InstantData> instants;

    auto lock() {
        return std::unique_lock{mutex};
    }

    auto getNewProfiledData() {
        auto lock = this->lock();
        threadFrameDatas.push_back(std::make_unique<ProfiledData>());
        return threadFrameDatas.back();
    }

    ~GlobalData() {
        /// Collect all data

        if (!shouldEnableProfiling) {
            return;
        }

        auto file = std::ofstream{"medit_profile_log.json"};
        file << "[\n";

        for (auto &data : threadFrameDatas) {
            // Output the thread's meta information

            file << "{";
            file << "\"name\": \"thread_name\", ";
            file << "\"ph\": \"M\", ";
            file << "\"pid\": 1, "; // Assuming process id is 1
            file << "\"tid\": " << data->id << ", ";
            file << "\"args\": {\"name\": \"" << data->name << "\"}";
            file << "}";

            for (auto &frame : data->frames) {
                file << frame << ",\n";
            }
        }

        file << "{}]\n";
        threadFrameDatas.clear();
    }

    GlobalData() = default;
    std::shared_ptr<GlobalData> globalData();
    std::mutex mutex{};
};

/// Should not be accessed a lot, could be performance hog due to static
std::shared_ptr<GlobalData> globalData() {
    /// This with the shared pointer is a workaround since we have no idea what
    /// is deinitialized first, but we want the global data to be the last
    static auto data = std::make_shared<GlobalData>();
    return data;
}

thread_local auto isThreadInitialized = true;

struct ThreadData {
    std::shared_ptr<GlobalData> global = globalData();
    std::shared_ptr<ProfiledData> data = global->getNewProfiledData();

    ThreadData() = default;

    ~ThreadData() {
        isThreadInitialized = false;
    }

    void enterFrame(std::string_view name,
                    std::string_view data2,
                    uint_least32_t line) {
        data->frames.push_back({
            .start = true,
            .name = name,
            .data2 = data2,
            .line = line,
            .tid = data->id,
        });
    }

    void exitFrame(std::string_view name,
                   std::string_view data2,
                   uint_least32_t line) {
        data->frames.push_back({
            .start = false,
            .name = name,
            .data2 = data2,
            .line = line,
            .tid = data->id,
        });
    }

    void instant(std::string_view data) {
        this->data->instants.push_back({
            .data = std::string{data},
            .tid = this->data->id,
        });
    }
};

thread_local auto localProfilingThreadData = ThreadData{};

} // namespace

ProfileDuration::ProfileDuration(std::string_view name,
                                 std::string_view data2,
                                 int line)
    : name{name}
    , data2{data2}
    , line{line} {
    if (!shouldEnableProfiling || !isThreadInitialized) {
        return;
    }

    localProfilingThreadData.enterFrame(name, data2, line);
}

ProfileDuration::~ProfileDuration() {
    if (!shouldEnableProfiling || !isThreadInitialized) {
        return;
    }

    localProfilingThreadData.exitFrame(name, data2, line);
}

void enableProfiling() {
    shouldEnableProfiling = true;
}

void profileInstant(std::string_view value) {
    if (!shouldEnableProfiling || !isThreadInitialized) {
        return;
    }
    localProfilingThreadData.instant(value);
}

void setProfilerThreadName(std::string name) {
    if (!shouldEnableProfiling || !isThreadInitialized) {
        return;
    }
    localProfilingThreadData.data->name = std::move(name);
}
