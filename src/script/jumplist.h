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

    std::vector<JumpPosition> _positions;

    void updatePosition(std::shared_ptr<Editor> currentEditor) {
        if (!_positions.empty()) {
            auto &p = _positions.back();
            if (p.isCommited) {
                if (p.editor.lock() == currentEditor &&
                    p.pos == currentEditor->cursor().pos()) {
                    return;
                }
            }
            if (p.editor.lock() == currentEditor &&
                p.pos.y() < currentEditor->cursor().pos().y() + 2 &&
                p.pos.y() > currentEditor->cursor().pos().y() - 2) {
                return;
            }
        }

        // TODO: First implementation. Update stuff
        _positions.push_back(JumpPosition{
            .editor = currentEditor,
            .path = currentEditor->path(),
            .pos = currentEditor->cursor().pos(),
        });
    }

    std::optional<JumpPosition> back() {
        if (_positions.size() < 2) {
            return std::nullopt;
        }
        _positions.pop_back();
        return _positions.back();
    }

    std::optional<JumpPosition> forward() {
        // Not implementedt
        return std::nullopt;
    }
};
