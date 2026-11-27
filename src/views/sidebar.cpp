#include "sidebar.h"
#include "screen/iscreen.h"
#include "script/ienvironment.h"
#include "syntax/palette.h"
#include "text/fstring.h"
#include <algorithm>
#include <cstdio>
#include <iterator>

namespace {

std::filesystem::path absolutePath(const std::filesystem::path &path) {
    return path.empty() ? path
                        : std::filesystem::absolute(path).lexically_normal();
}

// FChar currently represents a character, not a terminal column. Until screen
// buffers support grapheme widths, escape non-ASCII bytes in tree labels only.
std::string safeLabel(const std::string &name) {
    std::string result;
    for (const auto byte : name) {
        const auto c = static_cast<unsigned char>(byte);
        if (c >= 32 && c < 127 && c != '\\') {
            result += static_cast<char>(c);
        }
        else {
            char escaped[5];
            std::snprintf(escaped, sizeof(escaped), "\\x%02x", c);
            result += escaped;
        }
    }
    return result;
}

} // namespace

void SideBar::files(Path root, const std::vector<Path> &paths) {
    root = absolutePath(root);
    auto selection = _entries.empty() ? root : _entries[_current].path;
    if (root != _root.path) {
        selection = root;
        _current = 0;
        _expanded.clear();
        if (!root.empty())
            _expanded.insert(root);
        yScroll(0);
    }
    _root = Node{root};
    _directories.clear();
    if (!root.empty()) {
        _directories.insert(root);
        for (const auto &file : paths) {
            const auto relative = absolutePath(file).lexically_relative(root);
            // A settings file outside the project root cannot be shown here.
            if (relative.empty() || relative == "." || relative.is_absolute() ||
                *relative.begin() == "..")
                continue;
            auto *node = &_root;
            for (auto part = relative.begin(); part != relative.end(); ++part) {
                const auto childPath = node->path / *part;
                node = &node->children.try_emplace(*part, Node{childPath})
                            .first->second;
                if (std::next(part) != relative.end()) {
                    node->directory = true;
                    _directories.insert(childPath);
                }
                else if (node->children.empty()) {
                    node->directory = false;
                }
            }
        }
    }
    std::erase_if(_expanded, [this](const auto &path) {
        return !_directories.contains(path);
    });
    rebuildVisible(selection);
}

void SideBar::appendVisible(const Node &node, size_t depth) {
    _entries.push_back({node.path, node.directory, depth});
    if (!node.directory || !_expanded.contains(node.path))
        return;
    std::vector<const Node *> children;
    for (const auto &[name, child] : node.children)
        children.push_back(&child);
    std::sort(children.begin(), children.end(), [](auto a, auto b) {
        return a->directory != b->directory
                   ? a->directory > b->directory
                   : a->path.filename() < b->path.filename();
    });
    for (const auto *child : children)
        appendVisible(*child, depth + 1);
}

void SideBar::rebuildVisible(Path selection) {
    const auto previous = _current;
    _entries.clear();
    if (!_root.path.empty())
        appendVisible(_root, 0);
    contentHeight(_entries.size());
    while (!selection.empty()) {
        const auto found = std::find_if(
            _entries.begin(), _entries.end(), [&](const auto &entry) {
                return entry.path == selection;
            });
        if (found != _entries.end()) {
            select(static_cast<size_t>(found - _entries.begin()));
            return;
        }
        const auto parent = selection.parent_path();
        if (parent == selection)
            break;
        selection = parent;
    }
    select(previous);
}

void SideBar::select(size_t index) {
    _current = _entries.empty() ? 0 : std::min(index, _entries.size() - 1);
    const auto rows = height() > 1 ? height() - 1 : 0;
    if (!rows) {
        yScroll(0);
        return;
    }
    if (_current < yScroll())
        yScroll(_current);
    else if (_current - yScroll() >= rows)
        yScroll(_current - rows + 1);
    const auto maxScroll = _entries.size() > rows ? _entries.size() - rows : 0;
    yScroll(std::min(yScroll(), maxScroll));
}

void SideBar::onResize(size_t, size_t) {
    select(_current);
}

void SideBar::activate() {
    if (_entries.empty())
        return;
    const auto entry = _entries[_current];
    if (entry.directory) {
        if (!_expanded.erase(entry.path))
            _expanded.insert(entry.path);
        rebuildVisible(entry.path);
    }
    else if (_onOpen)
        _onOpen(entry.path);
}

bool SideBar::handleKey(KeyEvent key) {
    if (!key.state || key.hasModifiers())
        return false;
    if (key.key == Key::Text) {
        if (key.symbol == 'j')
            key.key = Key::Down;
        else if (key.symbol == 'k')
            key.key = Key::Up;
        else if (key.symbol == 'h')
            key.key = Key::Left;
        else if (key.symbol == 'l')
            key.key = Key::Right;
        else if (key.symbol == '\n' || key.symbol == '\r') {
            activate();
            return true;
        }
        else
            return false;
    }
    switch (key.key) {
    case Key::Up:
        select(_current > 0 ? _current - 1 : 0);
        return true;
    case Key::Down:
        select(_current + 1);
        return true;
    case Key::Left:
        if (!_entries.empty()) {
            const auto entry = _entries[_current];
            if (entry.directory && _expanded.erase(entry.path))
                rebuildVisible(entry.path);
            else
                rebuildVisible(entry.path.parent_path());
        }
        return true;
    case Key::Right:
        if (!_entries.empty() && _entries[_current].directory) {
            const auto path = _entries[_current].path;
            if (_expanded.insert(path).second)
                rebuildVisible(path);
            else if (_current + 1 < _entries.size() &&
                     _entries[_current + 1].depth > _entries[_current].depth)
                select(_current + 1);
        }
        return true;
    case Key::Home:
        select(0);
        return true;
    case Key::End:
        select(_entries.size());
        return true;
    default:
        return false;
    }
}

bool SideBar::keyPress(std::shared_ptr<IEnvironment> env) {
    return handleKey(env->key());
}

void SideBar::draw(IScreen &screen) {
    if (!visible() || !width() || !height())
        return;
    const auto contentWidth = width() - 1;
    for (size_t row = 0; row < height(); ++row) {
        if (contentWidth) {
            auto format = static_cast<FormatType>(Palette::standard);
            const auto index = row > 0 ? yScroll() + row - 1 : 0;
            if (row && index < _entries.size() && index == _current)
                format = Palette::currentLine;
            auto line = FString{contentWidth, FChar{' ', format}};
            std::string label;
            if (row == 0)
                label = " Project";
            else if (index < _entries.size()) {
                const auto &entry = _entries[index];
                label = index == _current ? "›" : " ";
                label += std::string(
                    std::min(entry.depth, contentWidth / 2) * 2, ' ');
                if (entry.directory)
                    label += _expanded.contains(entry.path) ? "▾ " : "▸ ";
                else
                    label += "  ";
                const auto name = entry.path.filename().string();
                label += safeLabel(name.empty() ? entry.path.string() : name);
            }
            const auto text = FString{label, format};
            const auto clipped = text.substr(0, contentWidth);
            std::copy(clipped.begin(), clipped.end(), line.begin());
            screen.draw(x(), y() + row, line);
        }
        screen.draw(
            x() + contentWidth, y() + row, FString{"│", Palette::lineNumbers});
    }
}

void SideBar::updateCursor(IScreen &screen) const {
    if (visible() && width() > 1 && height() > 1 && !_entries.empty() &&
        _current >= yScroll() && _current - yScroll() < height() - 1)
        screen.cursor(x(), y() + 1 + _current - yScroll());
    else if (visible() && width() > 1 && height() > 0)
        screen.cursor(x(), y());
}
