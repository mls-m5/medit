#pragma once

#include "keys/event.h"
#include "keys/ikeysink.h"
#include "views/scrollview.h"
#include <filesystem>
#include <functional>
#include <map>
#include <set>
#include <vector>

// Presentation of registered project files; never scans the filesystem.
class ProjectTreeView : public ScrollView, public IKeySink {
public:
    using Path = std::filesystem::path;
    struct Entry {
        Path path;
        bool directory = false;
        size_t depth = 0;
    };

    explicit ProjectTreeView(IView *parent);

    void files(Path root, const std::vector<Path> &files);
    const Path &root() const {
        return _root.path;
    }
    const auto &entries() const {
        return _entries;
    }
    size_t current() const {
        return _current;
    }
    void current(size_t index);
    bool expanded(const Path &path) const {
        return _expanded.contains(path);
    }
    bool reveal(const Path &path);

    void onOpen(std::function<void(const Path &)> callback) {
        _onOpen = std::move(callback);
    }
    void isOpen(std::function<bool(const Path &)> callback) {
        _isOpen = std::move(callback);
    }
    void focused(bool value) {
        _focused = value;
    }

    bool handleKey(KeyEvent key);
    bool keyPress(std::shared_ptr<IEnvironment> env) override;
    bool mouseDown(int x, int y) override;
    void draw(IScreen &screen) override;
    void updateCursor(IScreen &screen) const override;
    void onResize(size_t oldWidth, size_t oldHeight) override;

private:
    struct Node {
        Path path;
        bool directory = true;
        std::map<Path, Node> children;
    };
    Node _root;
    std::set<Path> _directories;
    std::set<Path> _expanded;
    std::vector<Entry> _entries;
    size_t _current = 0;
    bool _focused = false;
    std::function<void(const Path &)> _onOpen;
    std::function<bool(const Path &)> _isOpen;

    void appendVisible(const Node &node, size_t depth);
    void rebuildVisible(Path selection);
    void fitSelection();
    void activate();
    void left();
    void right();
};
