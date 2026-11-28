#include "files/diskcheck.h"
#include "files/file.h"
#include "files/ifilesystem.h"
#include "files/memoryfilesystem.h"
#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "text/cursorops.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace file_system_test {

std::vector<Path> sorted(std::vector<Path> paths) {
    std::sort(paths.begin(), paths.end());
    return paths;
}

struct TemporaryDirectory {
    Path path;

    TemporaryDirectory() {
        auto pattern =
            (std::filesystem::temp_directory_path() / "medit-fs-test-XXXXXX")
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

} // namespace file_system_test

TEST_SUIT_BEGIN(FileSystem)

TEST_CASE("the memory file system reads, writes, renames and lists") {
    auto fs = MemoryFileSystem{};
    EXPECT_TRUE(fs.write("/p/a.txt", "a"));
    EXPECT_TRUE(fs.write("/p/sub/b.txt", "b"));
    EXPECT_TRUE(fs.write("/p/build/CMakeCache.txt", ""));
    EXPECT_TRUE(fs.write("/p/build/c.cpp", ""));
    EXPECT_TRUE(fs.write("/other/d.txt", ""));

    ASSERT_EQ(*fs.read("/p/a.txt"), "a");
    EXPECT_FALSE(fs.read("/p/missing.txt"));
    EXPECT_TRUE(fs.stat("/p/sub").isDirectory);
    EXPECT_FALSE(fs.stat("/p/sub/b.txt").isDirectory);
    EXPECT_TRUE(fs.stat("/p/sub/b.txt").exists);
    EXPECT_FALSE(fs.stat("/p/nothing").exists);
    // A directory cannot be written as a file.
    EXPECT_FALSE(fs.write("/p/sub", "x"));

    EXPECT_FALSE(fs.rename("/p/a.txt", "/p/sub/b.txt"));
    EXPECT_TRUE(fs.rename("/p/a.txt", "/p/a2.txt"));
    EXPECT_FALSE(fs.stat("/p/a.txt").exists);

    auto filter = ListFilter{};
    filter.enterDirectory = [&fs](const Path &dir) {
        return !fs.stat(dir / "CMakeCache.txt").exists;
    };
    filter.includeFile = [](const Path &file) {
        return file.extension() == ".txt";
    };
    EXPECT_TRUE((file_system_test::sorted(fs.list("/p", filter)) ==
                 std::vector<Path>{"/p/a2.txt", "/p/sub/b.txt"}));
    filter.limit = 1;
    ASSERT_EQ(fs.list("/p", filter).size(), 1u);
}

TEST_CASE("the local file system lists without following directory links") {
    file_system_test::TemporaryDirectory dir;
    auto &fs = localFileSystem();
    const auto root = dir.path;
    std::filesystem::create_directories(root / "sub");
    EXPECT_TRUE(fs.write(root / "sub" / "a.txt", "a"));
    std::filesystem::create_directory_symlink(root / "sub", root / "link");
    std::filesystem::create_symlink(root / "sub" / "a.txt",
                                    root / "file-link.txt");

    EXPECT_TRUE(fs.stat(root / "link").isSymlink);
    EXPECT_TRUE(fs.stat(root / "link").isDirectory);
    EXPECT_FALSE(fs.stat(root / "sub").isSymlink);
    ASSERT_EQ(*fs.read(root / "file-link.txt"), "a");

    const auto files = file_system_test::sorted(fs.list(root, {}));
    // The file link is a file; the directory link is not entered.
    EXPECT_TRUE((files == std::vector<Path>{root / "file-link.txt",
                                            root / "sub" / "a.txt"}));

    EXPECT_FALSE(fs.rename(root / "sub" / "a.txt", root / "file-link.txt"));
    EXPECT_FALSE(fs.write(root / "sub", "x"));
}

TEST_CASE("files and buffers use the file system they are given") {
    auto fs = MemoryFileSystem{};
    fs.write("/p/a.txt", "1\n2\n");
    auto buffer = Buffer::open("/p/a.txt", fs);
    ASSERT_EQ(buffer->text(), "1\n2\n");

    fs.write("/p/a.txt", "1\ntwo\n");
    EXPECT_TRUE(buffer->file()->diskState() == DiskState::ChangedOnDisk);
    reloadFromDisk(*buffer);
    ASSERT_EQ(buffer->text(), "1\ntwo\n");

    insert(Cursor{*buffer, 0, 0}, FString{"x"});
    EXPECT_TRUE(buffer->save());
    ASSERT_EQ(*fs.read("/p/a.txt"), "x1\ntwo\n");

    fs.remove("/p/a.txt");
    EXPECT_TRUE(buffer->file()->diskState() == DiskState::DeletedOnDisk);
}

TEST_SUIT_END
