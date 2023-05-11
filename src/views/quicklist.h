#pragma once

#include "files/filesystem.h"
#include "views/editor.h"
#include "views/listview.h"

class QuickList : public Editor {
public:
    using PopulateRetT = std::vector<std::pair<FString, FString>>;
    using PopulateFunctionT = std::function<PopulateRetT()>;

    QuickList(IView *parent, PopulateFunctionT);
    ~QuickList() override;

    // @see IKeySink
    bool keyPress(std::shared_ptr<IEnvironment>) override;

    // @see IView
    void draw(IScreen &) override;

    void callback(std::function<void(filesystem::path)> f);

    void visible(bool value);

    bool visible() const;

    void updateList();

private:
    ListView _list;
    PopulateFunctionT _populate;
};
