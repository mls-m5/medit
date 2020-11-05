#pragma once

#include "keys/ikeysink.h"
#include "text/fstring.h"
#include "views/scrollview.h"
#include <any>
#include <vector>

class ListView : public ScrollView, IKeySink {
    struct ListItem;
    std::vector<ListItem> _lines;

public:
    ListView();
    ~ListView() override;

    void addLine(FString text, std::any dataContent);

    //! @see IView
    void draw(IScreen &) override;

    //! @see IKeySink
    bool keyPress(IEnvironment &env) override;

    //! @see IKeySink
    void updateCursor(IScreen &screen) const override;
};
