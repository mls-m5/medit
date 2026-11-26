#pragma once

#include "keys/event.h"
#include "keys/ikeysink.h"
#include "views/scrollview.h"
#include <filesystem>
#include <functional>
#include <map>
#include <set>
#include <vector>

// A snapshot of registered project files; no filesystem work while drawing.
class SideBar : public ScrollView, public IKeySink {
public:
    using Path = std::filesystem::path;
    struct Entry {
        Path path;
        bool directory;
        size_t depth;
    };

    explicit SideBar(IView *parent)
        : ScrollView{parent} {}

    void files(Path root, const std::vector<Path> &paths);
    void onOpen(std::function<void(const Path &)> callback) {
        _onOpen = std::move(callback);
    }
    const auto &entries() const {
        return _entries;
    }
    size_t current() const {
        return _current;
    }
    bool expanded(const Path &path) const {
        return _expanded.contains(path);
    }
    bool handleKey(KeyEvent key);
    bool keyPress(std::shared_ptr<IEnvironment> env) override;
    bool mouseDown(int, int) override {
        return false; // Mouse navigation is a later increment.
    }
    void draw(IScreen &screen) override;
    void updateCursor(IScreen &screen) const override;
    void onResize(size_t, size_t) override;

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
    std::function<void(const Path &)> _onOpen;

    void appendVisible(const Node &node, size_t depth);
    void rebuildVisible(Path selection);
    void select(size_t index);
    void activate();
};
