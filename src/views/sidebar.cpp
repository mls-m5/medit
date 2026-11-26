#include "sidebar.h"
#include "files/project.h"
#include "keys/event.h"
#include "screen/iscreen.h"
#include "script/ienvironment.h"
#include "syntax/palette.h"
#include "text/fstring.h"
#include "views/mainwindow.h"

SideBar::~SideBar() {
    if (_project)
        _project->unsubscribeFromFilesChanged(this);
}

void SideBar::project(Project &project) {
    if (_project)
        _project->unsubscribeFromFilesChanged(this);
    _project = &project;
    _tree.files(project.settings().root, project.files());
    project.subscribeToFilesChanged(
        [this] {
            _tree.files(_project->settings().root, _project->files());
            if (_followActiveFile && _pendingFollow)
                _pendingFollow = !_tree.reveal(_activeFile);
            if (window())
                window()->triggerRedraw();
        },
        this);
}

void SideBar::activeFile(std::filesystem::path path) {
    if (!path.empty())
        path = std::filesystem::absolute(path).lexically_normal();
    if (path == _activeFile)
        return;
    _activeFile = std::move(path);
    _pendingFollow =
        _followActiveFile && !_activeFile.empty() && !_tree.reveal(_activeFile);
}

void SideBar::toggleFollowActiveFile() {
    _followActiveFile = !_followActiveFile;
    _pendingFollow =
        _followActiveFile && !_activeFile.empty() && !_tree.reveal(_activeFile);
}

void SideBar::layoutTree() {
    _tree.x(x());
    _tree.y(y() + std::min(height(), size_t{2}));
    _tree.width(width() > 0 ? width() - 1 : 0);
    _tree.height(height() > 2 ? height() - 2 : 0);
}

void SideBar::onResize(size_t, size_t) {
    layoutTree();
}

bool SideBar::keyPress(std::shared_ptr<IEnvironment> env) {
    if (env->key() == KeyEvent{Key::Escape}) {
        env->mainWindow().focusEditor();
        return true;
    }
    return _tree.keyPress(env);
}

void SideBar::draw(IScreen &screen) {
    if (!visible() || width() == 0 || height() == 0)
        return;
    // A settings-only root change need not change Project::files().
    if (_project) {
        auto root = _project->settings().root;
        if (!root.empty())
            root = std::filesystem::absolute(root).lexically_normal();
        if (_tree.root() != root)
            _tree.files(root, _project->files());
    }
    layoutTree();
    const auto contentWidth = width() - 1;
    if (contentWidth > 0) {
        screen.draw(
            x(), y(), FString{contentWidth, FChar{' ', Palette::standard}});
        const auto title =
            FString{std::string{" Project [follow:"} +
                        (_followActiveFile ? "on]" : "off]"),
                    _focused ? Palette::currentLine : Palette::identifier};
        screen.draw(x(), y(), title.substr(0, contentWidth));
        if (height() > 1) {
            auto rule = FString{contentWidth, FChar{Utf8Char{"─"}}};
            for (auto &character : rule)
                character.f = Palette::lineNumbers;
            screen.draw(x(), y() + 1, rule);
        }
    }
    _tree.draw(screen);
    for (size_t row = 0; row < height(); ++row) {
        screen.draw(x() + contentWidth,
                    y() + row,
                    FString{row == 1 ? "┼" : "│", Palette::lineNumbers});
    }
}

void SideBar::updateCursor(IScreen &screen) const {
    if (!visible() || width() <= 1 || height() == 0)
        return;
    if (_tree.height() > 0 && !_tree.entries().empty())
        _tree.updateCursor(screen);
    else
        screen.cursor(x() + 1, y());
}

bool SideBar::contains(int mouseX, int mouseY) const {
    // The separator is not part of the tool's clickable area.
    return visible() && width() > 1 && mouseX >= 0 && mouseY >= 0 &&
           static_cast<size_t>(mouseX) >= x() &&
           static_cast<size_t>(mouseX) - x() < width() - 1 &&
           static_cast<size_t>(mouseY) >= y() &&
           static_cast<size_t>(mouseY) - y() < height();
}

bool SideBar::mouseDown(int mouseX, int mouseY) {
    if (!contains(mouseX, mouseY))
        return false;
    layoutTree();
    _tree.mouseDown(mouseX, mouseY);
    return true;
}
