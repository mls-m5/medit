#include "projecttreeview.h"
#include "files/path.h"
#include "screen/iscreen.h"
#include "script/ienvironment.h"
#include "syntax/palette.h"
#include "text/fstring.h"
#include "text/position.h"
#include <algorithm>
#include <cstdio>
#include <cwchar>
#include <iterator>
#include <locale.h>

namespace {

// IScreen's current buffers index FChars, not terminal columns. Keep labels
// single-cell in both backends: escape controls, invalid UTF-8, combining and
// double-width characters, without changing the actual file path.
std::string treeDisplayName(const std::string &name) {
    struct Utf8Locale {
        locale_t value = newlocale(LC_CTYPE_MASK, "C.UTF-8", nullptr);
        ~Utf8Locale() {
            if (value)
                freelocale(value);
        }
    };
    static const Utf8Locale locale;
    struct LocaleGuard {
        locale_t previous;
        ~LocaleGuard() {
            if (previous)
                uselocale(previous);
        }
    } guard{locale.value ? uselocale(locale.value) : nullptr};
    std::string result;
    for (size_t i = 0; i < name.size();) {
        wchar_t character = 0;
        std::mbstate_t state{};
        const auto count =
            locale.value
                ? std::mbrtowc(
                      &character, name.data() + i, name.size() - i, &state)
                : static_cast<size_t>(-1);
        char escaped[24];
        if (count == static_cast<size_t>(-1) ||
            count == static_cast<size_t>(-2) || count == 0) {
            const auto byte = static_cast<unsigned char>(name[i++]);
            if (byte >= 32 && byte < 127)
                result += static_cast<char>(byte);
            else {
                std::snprintf(escaped, sizeof(escaped), "\\x%02x", byte);
                result += escaped;
            }
        }
        else {
            if (::wcwidth(character) == 1)
                result.append(name, i, count);
            else {
                std::snprintf(escaped,
                              sizeof(escaped),
                              "\\u{%x}",
                              static_cast<unsigned int>(character));
                result += escaped;
            }
            i += count;
        }
    }
    return result;
}

std::filesystem::path treeAbsolutePath(const std::filesystem::path &path) {
    return normalizedAbsolutePath(path);
}

} // namespace

ProjectTreeView::ProjectTreeView(IView *parent)
    : ScrollView{parent} {}

void ProjectTreeView::files(Path root, const std::vector<Path> &files) {
    root = treeAbsolutePath(root);
    auto selection = _entries.empty() ? Path{} : _entries[_current].path;
    if (root != _root.path) {
        selection = root;
        _current = 0;
        _expanded.clear();
        if (!root.empty())
            _expanded.insert(root);
        scroll(0, 0);
    }
    _root = Node{root};
    _directories.clear();
    if (!root.empty()) {
        _directories.insert(root);
        for (const auto &file : files) {
            const auto path = treeAbsolutePath(file);
            const auto relative = path.lexically_relative(root);
            // Anonymous project settings may live outside the root. They are
            // registered files, but cannot be placed under this tree's root.
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
                else if (node->children.empty())
                    node->directory = false;
            }
        }
    }
    std::erase_if(_expanded, [this](const auto &path) {
        return !_directories.contains(path);
    });
    rebuildVisible(selection);
}

void ProjectTreeView::appendVisible(const Node &node, size_t depth) {
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

void ProjectTreeView::rebuildVisible(Path selection) {
    const auto previousIndex = _current;
    _entries.clear();
    if (!_root.path.empty())
        appendVisible(_root, 0);
    contentHeight(_entries.size());
    // Prefer the same row, then its closest still-visible parent.
    while (!selection.empty()) {
        const auto found = std::find_if(
            _entries.begin(), _entries.end(), [&](const auto &entry) {
                return entry.path == selection;
            });
        if (found != _entries.end()) {
            current(static_cast<size_t>(found - _entries.begin()));
            return;
        }
        const auto parent = selection.parent_path();
        if (parent == selection)
            break;
        selection = parent;
    }
    current(previousIndex);
}

void ProjectTreeView::fitSelection() {
    if (height() == 0 || _entries.empty()) {
        yScroll(0);
        return;
    }
    const auto lastScroll =
        _entries.size() > height() ? _entries.size() - height() : 0;
    yScroll(std::min(yScroll(), lastScroll));
    if (_current >= yScroll() && _current - yScroll() < height())
        return;
    fitPosition({0, _current});
    // ScrollView's preferred margin can exceed a tiny tree's height.
    const auto first = _current >= height() ? _current - height() + 1 : 0;
    yScroll(std::clamp(yScroll(), first, std::min(_current, lastScroll)));
}

void ProjectTreeView::current(size_t index) {
    _current = _entries.empty() ? 0 : std::min(index, _entries.size() - 1);
    fitSelection();
}

void ProjectTreeView::onResize(size_t, size_t) {
    fitSelection();
}

const ProjectTreeView::Node *ProjectTreeView::findNode(const Path &path) const {
    auto *node = &_root;
    const auto relative = treeAbsolutePath(path).lexically_relative(_root.path);
    if (_root.path.empty() || relative.empty() || relative.is_absolute() ||
        *relative.begin() == "..")
        return nullptr;
    if (relative != ".") {
        for (const auto &part : relative) {
            const auto child = node->children.find(part);
            if (child == node->children.end())
                return nullptr;
            node = &child->second;
        }
    }
    return node;
}

bool ProjectTreeView::containsFile(const Path &path) const {
    const auto *node = findNode(path);
    return node && !node->directory;
}

bool ProjectTreeView::reveal(const Path &path) {
    const auto normalized = treeAbsolutePath(path);
    // Follow must not add unregistered files or expand unrelated directories.
    if (!findNode(normalized))
        return false;
    for (auto parent = normalized.parent_path();
         _directories.contains(parent);) {
        _expanded.insert(parent);
        const auto next = parent.parent_path();
        if (next == parent)
            break;
        parent = next;
    }
    rebuildVisible(normalized);
    return true;
}

void ProjectTreeView::activate() {
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

void ProjectTreeView::left() {
    if (_entries.empty())
        return;
    const auto entry = _entries[_current];
    if (entry.directory && _expanded.erase(entry.path))
        rebuildVisible(entry.path);
    else
        rebuildVisible(entry.path.parent_path());
}

void ProjectTreeView::right() {
    if (_entries.empty() || !_entries[_current].directory)
        return;
    const auto path = _entries[_current].path;
    if (_expanded.insert(path).second)
        rebuildVisible(path);
    else if (_current + 1 < _entries.size() &&
             _entries[_current + 1].depth > _entries[_current].depth)
        current(_current + 1);
}

bool ProjectTreeView::handleKey(KeyEvent key) {
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
        current(_current > 0 ? _current - 1 : 0);
        return true;
    case Key::Down:
        current(_current + 1);
        return true;
    case Key::Left:
        left();
        return true;
    case Key::Right:
        right();
        return true;
    case Key::Home:
        current(0);
        return true;
    case Key::End:
        current(_entries.size());
        return true;
    case Key::PageUp:
        current(_current > height() ? _current - height() : 0);
        return true;
    case Key::PageDown:
        current(_current + std::max(height(), size_t{1}));
        return true;
    default:
        return false;
    }
}

bool ProjectTreeView::keyPress(std::shared_ptr<IEnvironment> env) {
    return handleKey(env->key());
}

bool ProjectTreeView::mouseDown(int mouseX, int mouseY) {
    if (!visible() || mouseX < 0 || mouseY < 0 ||
        static_cast<size_t>(mouseX) < x() ||
        static_cast<size_t>(mouseY) < y() ||
        static_cast<size_t>(mouseX) - x() >= width() ||
        static_cast<size_t>(mouseY) - y() >= height())
        return false;
    const auto index = yScroll() + static_cast<size_t>(mouseY) - y();
    if (index < _entries.size()) {
        current(index);
        activate();
    }
    return true;
}

void ProjectTreeView::draw(IScreen &screen) {
    if (!visible() || width() == 0 || height() == 0)
        return;
    for (size_t row = 0, index = yScroll(); row < height(); ++row, ++index) {
        auto format = static_cast<FormatType>(Palette::standard);
        const auto selected = index < _entries.size() && index == _current;
        if (selected)
            format = _focused ? Palette::currentLine : Palette::outside;
        auto line = FString{width(), FChar{' ', format}};
        if (index < _entries.size()) {
            const auto &entry = _entries[index];
            const auto open =
                !entry.directory && _isOpen && _isOpen(entry.path);
            auto label =
                std::string{selected ? "›" : " "} + (open ? "● " : "  ");
            // Bound indentation before allocating for deeply nested projects.
            label += std::string(std::min(entry.depth, width() / 2) * 2, ' ');
            if (entry.directory)
                label += expanded(entry.path) ? "▾ " : "▸ ";
            auto name = entry.path.filename().string();
            if (name.empty())
                name = entry.path.string();
            label += treeDisplayName(name);
            if (entry.directory && entry.path.has_filename())
                label += '/';
            const auto changed =
                !entry.directory && _gitChanged && _gitChanged(entry.path);
            auto text = FString{label,
                                changed           ? Palette::warning
                                : selected        ? format
                                : entry.directory ? Palette::identifier
                                : open            ? Palette::string
                                                  : Palette::standard};
            const auto clipped = text.substr(0, width());
            std::copy(clipped.begin(), clipped.end(), line.begin());
        }
        screen.draw(x(), y() + row, line);
    }
}

void ProjectTreeView::updateCursor(IScreen &screen) const {
    if (visible() && width() > 0 && height() > 0 && !_entries.empty() &&
        _current >= yScroll() && _current - yScroll() < height())
        screen.cursor(x(), y() + _current - yScroll());
}
