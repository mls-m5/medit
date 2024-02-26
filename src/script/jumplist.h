#pragma once

#include "text/position.h"
#include "views/editor.h"
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

struct JumpList {
    JumpList() = default;
    ~JumpList() = default;
    JumpList(const JumpList &) = delete;
    JumpList(JumpList &&) = delete;
    JumpList &operator=(const JumpList &) = delete;
    JumpList &operator=(JumpList &&) = delete;

    struct JumpPosition {
        std::weak_ptr<Editor> editor;
        std::filesystem::path path;
        Position pos;
        bool isCommited = false;
    };

    void updatePosition(std::shared_ptr<Editor> currentEditor) {
        if (!_positions.empty()) {
            auto &p = currentPosition();
            if (p.isCommited) {
                if (p.editor.lock() == currentEditor &&
                    p.pos == currentEditor->cursor().pos()) {
                    return;
                }
            }
            if (p.editor.lock() == currentEditor &&
                p.pos.y() < currentEditor->cursor().pos().y() + 2 &&
                p.pos.y() > currentEditor->cursor().pos().y() - 2) {
                p.pos = currentEditor->cursor().pos();
                return;
            }
        }

        ++_currentIndex;

        auto p = JumpPosition{
            .editor = currentEditor,
            .path = currentEditor->path(),
            .pos = currentEditor->cursor().pos(),
        };

        if (_currentIndex < _positions.size()) {
            _positions.at(_currentIndex) = p;
        }
        else {
            _positions.push_back(p);
        }
    }

    std::optional<JumpPosition> back() {
        if (_currentIndex <= 0) {
            return std::nullopt;
        }

        if (_positions.size() < 2) {
            return std::nullopt;
        }

        --_currentIndex;

        return _positions.at(_currentIndex);
    }

    std::optional<JumpPosition> forward() {
        if (_currentIndex >= _positions.size() - 1) {
            return std::nullopt;
        }

        ++_currentIndex;

        return _positions.at(_currentIndex);
    }

private:
    JumpPosition &currentPosition() {
        if (_positions.empty()) {
            return _positions.emplace_back();
        }

        if (_currentIndex >= _positions.size()) {
            _currentIndex = _positions.size() - 1;
        }

        return _positions.at(_currentIndex);
    }

    std::vector<JumpPosition> _positions;
    size_t _currentIndex = 0;
};
