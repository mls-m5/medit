#include "core/coreenvironment.h"
#include "core/itimer.h"
#include "core/jobqueue.h"
#include "core/logtype.h"
#include "files/unsavablefile.h"
#include "helpers/temporaryproject.h"
#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include <chrono>
#include <fstream>
#include <map>

namespace autosave_test {
struct Timer : ITimer {
    std::map<size_t, std::function<void()>> pending;
    DurationT lastDelay{};
    size_t next = 0;
    size_t setTimeout(DurationT delay,
                      std::function<void()> callback) override {
        lastDelay = delay;
        pending.emplace(++next, std::move(callback));
        return next;
    }
    void cancel(size_t ref) override {
        pending.erase(ref);
    }
    void stop() override {
        pending.clear();
    }
    void loop() override {}
    void start() override {}
    void fire() {
        auto callbacks = std::move(pending);
        pending.clear();
        for (auto &[ref, callback] : callbacks)
            callback();
    }
};
struct Fixture {
    test_helpers::TemporaryProject project;
    JobQueue jobs;
    JobQueue gui;
    Timer timer;
    ThreadContext context{jobs, gui, timer};
    CoreEnvironment core{context, project.path(".medit.json")};
    std::string disk(const std::filesystem::path &path) {
        std::ifstream file{project.path(path)};
        return {std::istreambuf_iterator<char>{file},
                std::istreambuf_iterator<char>{}};
    }
};
} // namespace autosave_test

TEST_SUIT_BEGIN(Autosave)

TEST_CASE("modified named buffers save after two seconds on GUI queue") {
    autosave_test::Fixture f;
    f.project.write("one.cpp", "old\n");
    auto buffer = f.core.files().open(f.project.path("one.cpp"));
    buffer->text(std::string{"new\n"});
    EXPECT_TRUE(f.timer.lastDelay == std::chrono::seconds{2});
    EXPECT_TRUE(buffer->isChanged());
    f.timer.fire();
    EXPECT_TRUE(buffer->isChanged());
    ASSERT_EQ(f.disk("one.cpp"), "old\n");
    f.gui.work(false);
    EXPECT_FALSE(buffer->isChanged());
    ASSERT_EQ(f.disk("one.cpp"), "new\n");
}

TEST_CASE("new edits replace pending saves and a manual save cancels them") {
    autosave_test::Fixture f;
    f.project.write("one.cpp", "old\n");
    auto buffer = f.core.files().open(f.project.path("one.cpp"));
    buffer->text(std::string{"first\n"});
    const auto old = f.timer.pending.begin()->second;
    buffer->text(std::string{"second\n"});
    old(); // Already dispatched to the GUI queue when the next edit arrives.
    f.gui.work(false);
    ASSERT_EQ(f.disk("one.cpp"), "old\n");
    EXPECT_TRUE(buffer->save());
    f.timer.fire();
    f.gui.work(false);
    ASSERT_EQ(f.disk("one.cpp"), "second\n");
}

TEST_CASE("diagnostic redraws do not postpone a pending save") {
    autosave_test::Fixture f;
    f.project.write("one.cpp", "old\n");
    auto buffer = f.core.files().open(f.project.path("one.cpp"));
    buffer->text(std::string{"new\n"});
    const auto scheduled = f.timer.pending.begin()->first;
    buffer->emitChangeSignal();
    EXPECT_TRUE(f.timer.pending.contains(scheduled));
    f.timer.fire();
    f.gui.work(false);
    ASSERT_EQ(f.disk("one.cpp"), "new\n");
}

TEST_CASE("unnamed and temporary buffers are not autosaved") {
    autosave_test::Fixture f;
    auto unnamed = f.core.files().create();
    unnamed->text(std::string{"draft"});
    auto temporary = f.core.files().create();
    temporary->assignFile(std::make_unique<UnsavableFile>("Console"));
    temporary->text(std::string{"log"});
    EXPECT_TRUE(f.timer.pending.empty());
}

TEST_CASE("external change skips autosave without overwriting the disk") {
    autosave_test::Fixture f;
    f.project.write("one.cpp", "original\n");
    auto buffer = f.core.files().open(f.project.path("one.cpp"));
    const auto before =
        std::filesystem::last_write_time(f.project.path("one.cpp"));
    buffer->text(std::string{"editor\n"});
    f.project.write("one.cpp", "external\n");
    std::filesystem::last_write_time(f.project.path("one.cpp"),
                                     before + std::chrono::seconds{2});
    int warnings = 0;
    f.core.subscribeToLogCallback(
        [&](LogType type, std::string) {
            if (type == LogType::Error)
                ++warnings;
        },
        &warnings);
    f.timer.fire();
    f.gui.work(false);
    ASSERT_EQ(f.disk("one.cpp"), "external\n");
    EXPECT_TRUE(buffer->isChanged());
    ASSERT_EQ(warnings, 1);
    EXPECT_TRUE(f.timer.pending.empty());
    f.core.unsubscribeToConsoleCallback(&warnings);
}

TEST_SUIT_END
