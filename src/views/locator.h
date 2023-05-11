#pragma once

#include "files/project.h"
#include "quicklist.h"

class Locator : public QuickList {
public:
    Locator(IView *parent, Project &projectFiles);
    ~Locator() override;

    Project &_project;

private:
    PopulateRetT populate() const;
};
