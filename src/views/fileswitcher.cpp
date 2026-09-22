#include "views/fileswitcher.h"
#include "views/quicklist.h"

FileSwitcher::FileSwitcher(IView *parent, Project &projectFiles)
    : QuickList{parent, [this]() { return populate(); }} {}

FileSwitcher::~FileSwitcher() {}

QuickList::PopulateRetT FileSwitcher::populate() const {
    return {};
}
