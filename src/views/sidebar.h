#pragma once

#include "keys/ikeysink.h"
#include "views/projecttreeview.h"
#include "views/view.h"

class IScreen;
class Project;

// A window region for tools, independent of the text-editor split list.
class SideBar : public View, public IKeySink {
    bool _focused = false;
    bool _followActiveFile = true;
    bool _pendingFollow = false;
    Project *_project = nullptr;
    std::filesystem::path _activeFile;
    ProjectTreeView _tree{this};

    void layoutTree();
    void updateProjectFiles();

public:
    explicit SideBar(IView *parent)
        : View{parent} {
        layoutTree();
    }
    ~SideBar() override;

    // Project must outlive the sidebar. Subscription and updates use the GUI
    // thread.
    void project(Project &project);
    ProjectTreeView &tree() {
        return _tree;
    }
    const ProjectTreeView &tree() const {
        return _tree;
    }
    void activeFile(std::filesystem::path path);
    bool followActiveFile() const {
        return _followActiveFile;
    }
    void toggleFollowActiveFile();
    bool contains(int x, int y) const;
    void onResize(size_t oldWidth, size_t oldHeight) override;

    void draw(IScreen &screen) override;

    void focused(bool value) {
        _focused = value;
        _tree.focused(value);
    }
    bool focused() const {
        return _focused;
    }

    bool keyPress(std::shared_ptr<IEnvironment> env) override;
    void updateCursor(IScreen &screen) const override;
    bool mouseDown(int x, int y) override;
};
