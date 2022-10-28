#pragma once

#include "files/file.h"
#include "files/path.h"
#include "rawbuffer.h"
#include "text/history.h"
#include <filesystem>
#include <memory>

// Text stored in memory with assosiated undo/redo-history and associated
// filename information
class Buffer : public RawBuffer {
public:
    using RawBuffer::RawBuffer;

    History &history() {
        return _history;
    }

    [[nodiscard]] static std::unique_ptr<Buffer> open(
        std::filesystem::path path) {
        auto buffer = std::make_unique<Buffer>();

        auto file = std::make_unique<File>(path);
        if (std::filesystem::exists(file->path())) {
            file->load(*buffer);
        }
        buffer->_file = std::move(file);
        return buffer;
    }

    void save() {
        if (_file) {
            _file->save(*this);
            isChanged(false);
        }
    }

    void load() {
        if (_file) {
            _file->load(*this);
            isChanged(false);
        }
    }

    Path path() {
        if (_file) {
            return _file->path();
        }
        else {
            return {};
        }
    }

    auto *file() {
        return _file.get();
    }

private:
    History _history{*this};
    std::unique_ptr<IFile> _file;
};
