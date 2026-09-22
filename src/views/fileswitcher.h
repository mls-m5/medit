#pragma once

#include "quicklist.h"

class FileSwitcher : public QuickList {
public:
    FileSwitcher(const FileSwitcher &) = delete;
    FileSwitcher(FileSwitcher &&) = delete;
    FileSwitcher &operator=(const FileSwitcher &) = delete;
    FileSwitcher &operator=(FileSwitcher &&) = delete;

    FileSwitcher(IView *parent, Project &projectFiles);
    ~FileSwitcher() override;

private:
    PopulateRetT populate() const;
};
