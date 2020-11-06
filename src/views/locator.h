#pragma once

#include "files/filesystem.h"
#include "files/projectfiles.h"
#include "script/environment.h"
#include "views/editor.h"
#include "views/listview.h"

class Locator : public Editor {
public:
    Environment _env;
    ListView _list;
    ProjectFiles _projectFiles;
    std::vector<filesystem::path> _fileCache;

    Locator(IEnvironment &env);
    ~Locator() override;

    // @see IKeySink
    bool keyPress(IEnvironment &) override;

    // @see IView
    void draw(IScreen &) override;

    void callback(std::function<void(filesystem::path)> f);

    void visible(bool value);

    void updateCache(filesystem::path pathInProject);
    void updateList();
};
