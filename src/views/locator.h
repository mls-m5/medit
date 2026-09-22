#pragma once

#include "files/project.h"
#include "quicklist.h"

class Locator : public QuickList {
public:
    Locator(const Locator &) = delete;
    Locator(Locator &&) = delete;
    Locator &operator=(const Locator &) = delete;
    Locator &operator=(Locator &&) = delete;

    Locator(IView *parent, Project &projectFiles);
    ~Locator() override;

    Project &_project;

private:
    PopulateRetT populate() const;
};
