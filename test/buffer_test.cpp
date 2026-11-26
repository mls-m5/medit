#include "files/file.h"
#include "helpers/temporaryproject.h"
#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include <chrono>
#include <string_view>
#include <thread>

using namespace std::literals;

TEST_SUIT_BEGIN(Buffer)

TEST_CASE("failed file save keeps the buffer marked changed") {
    test_helpers::TemporaryProject project;
    Buffer buffer{"changed"};
    buffer.isChanged(true);
    buffer.assignFile(std::make_unique<File>(project.path("missing/file.txt")));
    EXPECT_FALSE(buffer.save());
    EXPECT_TRUE(buffer.isChanged());
}

TEST_CASE("external changes block saving until explicitly overwritten") {
    test_helpers::TemporaryProject project;
    project.write("example.cpp", "original\n");
    const auto path = project.path("example.cpp");
    const auto originalTime = std::filesystem::last_write_time(path);
    auto buffer = Buffer::open(path);
    buffer->text(std::string{"editor version\n"});
    project.write("example.cpp", "external version\n");
    std::filesystem::last_write_time(path,
                                     originalTime + std::chrono::seconds{2});
    EXPECT_TRUE(buffer->hasExternalChanges());
    EXPECT_FALSE(buffer->save());
    EXPECT_TRUE(buffer->isChanged());
    std::ifstream disk{project.path("example.cpp")};
    std::string line;
    std::getline(disk, line);
    ASSERT_EQ(line, "external version");
    EXPECT_TRUE(buffer->save(true));
    EXPECT_FALSE(buffer->hasExternalChanges());
    EXPECT_FALSE(buffer->isChanged());
}

TEST_CASE(
    "size changes are detected even when modification time is preserved") {
    test_helpers::TemporaryProject project;
    project.write("example.cpp", "short\n");
    const auto path = project.path("example.cpp");
    const auto time = std::filesystem::last_write_time(path);
    auto buffer = Buffer::open(path);
    buffer->text(std::string{"editor\n"});
    project.write("example.cpp", "longer external contents\n");
    std::filesystem::last_write_time(path, time);
    EXPECT_TRUE(buffer->hasExternalChanges());
    EXPECT_FALSE(buffer->save());
    std::ifstream disk{path};
    std::string line;
    std::getline(disk, line);
    ASSERT_EQ(line, "longer external contents");
}

#if defined(__linux__) && !defined(__EMSCRIPTEN__)
TEST_CASE("ctime detects equal-size edits with preserved modification time") {
    test_helpers::TemporaryProject project;
    project.write("example.cpp", "original\n");
    const auto path = project.path("example.cpp");
    const auto time = std::filesystem::last_write_time(path);
    auto buffer = Buffer::open(path);
    buffer->text(std::string{"in editor\n"});
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
    project.write("example.cpp", "external\n");
    std::filesystem::last_write_time(path, time);
    EXPECT_TRUE(buffer->hasExternalChanges());
    EXPECT_FALSE(buffer->save());
}

TEST_CASE("replacing a file with equal size and time still blocks saving") {
    test_helpers::TemporaryProject project;
    project.write("example.cpp", "original\n");
    const auto path = project.path("example.cpp");
    const auto time = std::filesystem::last_write_time(path);
    auto buffer = Buffer::open(path);
    buffer->text(std::string{"in editor\n"});
    const auto replacement = project.path("replacement.cpp");
    project.write("replacement.cpp", "external\n");
    std::filesystem::last_write_time(replacement, time);
    std::filesystem::rename(replacement, path);
    EXPECT_TRUE(buffer->hasExternalChanges());
    EXPECT_FALSE(buffer->save());
    std::ifstream disk{path};
    std::string line;
    std::getline(disk, line);
    ASSERT_EQ(line, "external");
    EXPECT_TRUE(buffer->save(true));
    EXPECT_FALSE(buffer->hasExternalChanges());
}
#endif

TEST_CASE("an unchanged empty file can be saved") {
    test_helpers::TemporaryProject project;
    project.write("empty.cpp", "");
    auto buffer = Buffer::open(project.path("empty.cpp"));
    buffer->text(std::string{"new contents\n"});
    EXPECT_FALSE(buffer->hasExternalChanges());
    EXPECT_TRUE(buffer->save());
    EXPECT_FALSE(buffer->hasExternalChanges());
}

TEST_CASE("a removed backing file is an external change") {
    test_helpers::TemporaryProject project;
    project.write("example.cpp", "original\n");
    auto buffer = Buffer::open(project.path("example.cpp"));
    buffer->text(std::string{"editor version\n"});
    std::filesystem::remove(project.path("example.cpp"));
    EXPECT_TRUE(buffer->hasExternalChanges());
    EXPECT_FALSE(buffer->save());
    EXPECT_FALSE(std::filesystem::exists(project.path("example.cpp")));
}

TEST_CASE("instantiate") {
    Buffer buffer;
}

TEST_CASE("fill with text") {
    const std::string_view testText = "apa bepa\n bearne";

    Buffer buffer{testText};

    auto returnText = buffer.text();

    ASSERT_EQ(returnText, testText);
    ASSERT_EQ(returnText.size(), testText.size());
}

// TEST_CASE("insert string single line") {
//     const std::string_view testText = "apa bepa";
//     {
//         Buffer buffer{"apa", true};

////        insert(buffer.begin(), testText);
//        buffer.insert(0, testText);

//        ASSERT_EQ(buffer.text(), "apa bepaapa");
//    }
//    {
//        Buffer buffer{"apa", true};

//        buffer.insert(1, testText);

//        ASSERT_EQ(buffer.text(), "apaapa bepa");
//    }
//}

TEST_CASE("apply text change") {
    {
        auto testText = "hello there"sv;
        auto resultText = "wazzup?"sv;

        Buffer buffer{testText};

        buffer.apply(BufferEdit{
            "hello there",
            "wazzup?",
            buffer.cursor({0, 0}),
        });

        auto returnText = buffer.text();

        ASSERT_EQ(returnText, resultText);
        ASSERT_EQ(returnText.size(), resultText.size());
    }

    {
        const std::string_view testText = "apa bepa\n bearne";
        const std::string_view resultText = "bepa bepa\n bearne";

        Buffer buffer{testText};

        buffer.apply(BufferEdit{
            "apa",
            "bepa",
            buffer.cursor({0, 0}),
        });

        auto returnText = buffer.text();

        ASSERT_EQ(returnText, resultText);
        ASSERT_EQ(returnText.size(), resultText.size());
    }
}

TEST_CASE("apply text change - multiline") {

    {
        const std::string_view testText = "apa bepa\n bearne";
        const std::string_view resultText = "ax bearne";

        Buffer buffer{testText};

        buffer.apply(BufferEdit{
            "pa bepa\n",
            "x",
            buffer.cursor({1, 0}),
        });

        auto returnText = buffer.text();

        ASSERT_EQ(returnText, resultText);
        ASSERT_EQ(returnText.size(), resultText.size());
    }

    {
        const std::string_view testText = "apa bepa\n bearne";
        const std::string_view resultText = "x bearne";

        Buffer buffer{testText};

        buffer.apply(BufferEdit{
            "apa bepa\n",
            "x",
            buffer.cursor({0, 0}),
        });

        auto returnText = buffer.text();

        ASSERT_EQ(returnText, resultText);
        ASSERT_EQ(returnText.size(), resultText.size());
    }
}

TEST_SUIT_END
