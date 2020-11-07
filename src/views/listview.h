#pragma once

#include "keys/ikeysink.h"
#include "text/fstring.h"
#include "views/scrollview.h"
#include <any>
#include <functional>
#include <vector>

class ListView : public ScrollView, public IKeySink {
    struct ListItem;
    std::vector<ListItem> _lines;
    std::function<void(const std::string &, size_t, const std::any &)>
        _callback;
    size_t _current = 0;

public:
    ListView();
    ~ListView() override;

    void addLine(FString text, std::any dataContent = {});
    void clear();

    //! What function to call when list item is selected
    void callback(
        std::function<void(const std::string &, size_t, const std::any &)> f) {
        _callback = std::move(f);
    }

    //! @see IView
    void draw(IScreen &) override;

    //! @see IKeySink
    bool keyPress(IEnvironment &env) override;

    //! @see IKeySink
    void updateCursor(IScreen &screen) const override;

    void fitCursor() {}

    void current(size_t value);

    auto current() const {
        return _current;
    }
};
