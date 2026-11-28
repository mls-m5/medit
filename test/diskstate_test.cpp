#include "files/diskcheck.h"
#include "files/file.h"
#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "text/cursorops.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <stdexcept>
#include <string>

namespace disk_state_test {

struct TemporaryDirectory {
    std::filesystem::path path;

    TemporaryDirectory() {
        auto pattern =
            (std::filesystem::temp_directory_path() / "medit-disk-test-XXXXXX")
                .string();
        if (!mkdtemp(pattern.data())) {
            throw std::runtime_error("Cannot create temporary directory");
        }
        path = pattern;
    }

    ~TemporaryDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
};

void write(const std::filesystem::path &path, const std::string &content) {
    std::ofstream{path, std::ios::binary} << content;
}

std::string read(const std::filesystem::path &path) {
    auto file = std::ifstream{path, std::ios::binary};
    return {std::istreambuf_iterator<char>{file},
            std::istreambuf_iterator<char>{}};
}

/// Types `text` at the start of line `y`, as the user would.
void type(Buffer &buffer, size_t y, const std::string &text) {
    insert(Cursor{buffer, 0, y}, FString{text});
}

} // namespace disk_state_test

TEST_SUIT_BEGIN(DiskState)

TEST_CASE("a loaded file is in sync until someone else changes it") {
    disk_state_test::TemporaryDirectory dir;
    const auto path = dir.path / "a.txt";
    disk_state_test::write(path, "one\ntwo\n");
    auto buffer = Buffer::open(path);
    EXPECT_TRUE(buffer->file()->diskState() == DiskState::InSync);

    disk_state_test::write(path, "one\nTWO\n");
    EXPECT_TRUE(buffer->file()->diskState() == DiskState::ChangedOnDisk);

    // Written back with the same content: nothing changed.
    disk_state_test::write(path, "one\ntwo\n");
    EXPECT_TRUE(buffer->file()->diskState() == DiskState::InSync);

    std::filesystem::remove(path);
    EXPECT_TRUE(buffer->file()->diskState() == DiskState::DeletedOnDisk);
}

TEST_CASE("our own save is not a change on disk") {
    disk_state_test::TemporaryDirectory dir;
    const auto path = dir.path / "a.txt";
    disk_state_test::write(path, "one\n");
    auto buffer = Buffer::open(path);
    disk_state_test::type(*buffer, 0, "zero ");
    EXPECT_TRUE(buffer->save());
    ASSERT_EQ(disk_state_test::read(path), "zero one\n");
    EXPECT_TRUE(buffer->file()->diskState() == DiskState::InSync);
    EXPECT_FALSE(buffer->isChanged());
}

TEST_CASE("saving refuses to overwrite someone else's change") {
    disk_state_test::TemporaryDirectory dir;
    const auto path = dir.path / "a.txt";
    disk_state_test::write(path, "one\n");
    auto buffer = Buffer::open(path);
    disk_state_test::type(*buffer, 0, "mine ");
    disk_state_test::write(path, "theirs\n");

    EXPECT_FALSE(buffer->save());
    ASSERT_EQ(disk_state_test::read(path), "theirs\n");
    EXPECT_TRUE(buffer->isChanged());

    EXPECT_TRUE(buffer->save(true));
    ASSERT_EQ(disk_state_test::read(path), "mine one\n");
    EXPECT_TRUE(buffer->file()->diskState() == DiskState::InSync);
}

TEST_CASE("saving writes a file deleted on disk again") {
    disk_state_test::TemporaryDirectory dir;
    const auto path = dir.path / "a.txt";
    disk_state_test::write(path, "one\n");
    auto buffer = Buffer::open(path);
    std::filesystem::remove(path);
    EXPECT_TRUE(buffer->save());
    ASSERT_EQ(disk_state_test::read(path), "one\n");
}

TEST_CASE("checkDisk marks the conflicting lines and clears them again") {
    disk_state_test::TemporaryDirectory dir;
    const auto path = dir.path / "a.txt";
    disk_state_test::write(path, "1\n2\n3\n4\n5\n6\n");
    auto buffer = Buffer::open(path);
    disk_state_test::type(*buffer, 4, "mine "); // Line 5.
    disk_state_test::write(path, "1\n2\n3\n4\ntheirs\n6\n");

    EXPECT_TRUE(checkDisk(*buffer) == DiskState::ChangedOnDisk);
    EXPECT_TRUE(buffer->diskState() == DiskState::ChangedOnDisk);
    auto &marks = buffer->diagnostics();
    EXPECT_TRUE(marks.findLineDiagnostic(4));
    EXPECT_FALSE(marks.findLineDiagnostic(0));
    ASSERT_EQ(marks.findLineDiagnostic(4)->source, "disk");

    EXPECT_TRUE(buffer->save(true));
    EXPECT_TRUE(buffer->diskState() == DiskState::InSync);
    EXPECT_FALSE(buffer->diagnostics().findLineDiagnostic(4));
}

TEST_CASE("a change that does not conflict marks the first line") {
    disk_state_test::TemporaryDirectory dir;
    const auto path = dir.path / "a.txt";
    disk_state_test::write(path, "1\n2\n3\n4\n5\n6\n");
    auto buffer = Buffer::open(path);
    disk_state_test::write(path, "1\n2\n3\n4\n5\nsix\n");
    EXPECT_TRUE(checkDisk(*buffer) == DiskState::ChangedOnDisk);
    EXPECT_TRUE(buffer->diagnostics().findLineDiagnostic(0));
    ASSERT_EQ(buffer->diagnostics().findLineDiagnostic(0)->message,
              "changed on disk");
}

TEST_CASE("reloading replaces only the changed lines and can be undone") {
    disk_state_test::TemporaryDirectory dir;
    const auto path = dir.path / "a.txt";
    disk_state_test::write(path, "one\ntwo\nthree\n");
    auto buffer = Buffer::open(path);
    disk_state_test::type(*buffer, 0, "mine ");
    const auto mine = buffer->text();

    disk_state_test::write(path, "one\n2\nthree\nfour\n");
    buffer->load();
    ASSERT_EQ(buffer->text(), "one\n2\nthree\nfour\n");
    EXPECT_FALSE(buffer->isChanged());
    EXPECT_TRUE(buffer->file()->diskState() == DiskState::InSync);

    buffer->history().undoMajor();
    ASSERT_EQ(buffer->text(), mine);
}

TEST_CASE("replaceText gives exactly the new text") {
    auto random = std::mt19937{42};
    auto text = [&random] {
        auto ret = std::string{};
        const auto lines = random() % 6;
        for (size_t i = 0; i < lines; ++i) {
            if (i) {
                ret += '\n';
            }
            const auto length = random() % 3;
            for (size_t j = 0; j < length; ++j) {
                ret += static_cast<char>('a' + random() % 3);
            }
        }
        if (random() % 2) {
            ret += "\nå";
        }
        return ret;
    };
    for (int i = 0; i < 1000; ++i) {
        const auto from = text();
        const auto to = text();
        auto buffer = Buffer{FString{from}};
        ASSERT_EQ(buffer.text(), from);
        buffer.replaceText(to);
        ASSERT_EQ(buffer.text(), to);
        if (from != to) {
            buffer.history().undoMajor();
            ASSERT_EQ(buffer.text(), from);
        }
    }
}

TEST_CASE("the saved bytes end with a newline") {
    ASSERT_EQ(fileBytes(Buffer{"a\nb"}), "a\nb\n");
    ASSERT_EQ(fileBytes(Buffer{"a\nb\n"}), "a\nb\n");
}

TEST_SUIT_END
