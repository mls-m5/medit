#pragma once

#include "files/filesystem.h"
#include "files/project.h"
#include "script/scope.h"
#include "views/editor.h"
#include "views/listview.h"

class Locator : public Editor {
public:
    //    Environment _env;
    ListView _list;
    Project &_projectFiles;

    Locator(IView *parent, Project &projectFiles);
    ~Locator() override;

    // @see IKeySink
    bool keyPress(std::shared_ptr<IEnvironment>) override;

    // @see IView
    void draw(IScreen &) override;

    void callback(std::function<void(filesystem::path)> f);

    void visible(bool value);

    bool visible() const;

    void updateList();
};
