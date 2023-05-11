#include "quicklist.h"
#include "script/ienvironment.h"
#include "text/buffer.h"
#include "text/utf8caseconversion.h"
#include <filesystem>
#include <string>

QuickList::QuickList(IView *parent, PopulateFunctionT populateFunction)
    : Editor{parent, std::make_shared<Buffer>(true)}
    , _list{parent}
    , _populate{populateFunction} {
    _list.x(0);
    _list.y(1);
    _list.width(20);
    _list.height(20);
}

bool QuickList::keyPress(std::shared_ptr<IEnvironment> env) {
    if (_list.keyPress(env)) {
        return true;
    }
    else {
        auto ret = Editor::keyPress(env);

        updateList();
        return ret;
    }
}

QuickList::~QuickList() = default;

void QuickList::draw(IScreen &screen) {
    if (Editor::visible()) {
        Editor::draw(screen);
        _list.draw(screen);
    }
}

void QuickList::callback(std::function<void(filesystem::path)> f) {
    _list.callback([f, this](auto &&, size_t, auto &&path) {
        if (path.has_value()) {
            f(std::any_cast<std::string>(path));
        }
        else {
            f({});
        }
        buffer().clear();
    });
}

void QuickList::visible(bool value) {
    _list.visible(value);
    Editor::visible(value);

    if (value) {
        updateList();
    }
}

bool QuickList::visible() const {
    return _list.visible();
}

void QuickList::updateList() {
    const size_t maxFillLen = 20;
    _list.clear();

    auto bufferStr = std::string(toLower(FString{buffer().text()}));

    for (auto &item : _populate()) {
        size_t fillLen = 1;
        //        auto str = std::string{fstring};
        auto str = std::string{toLower(item.second)};

        if (str.find(bufferStr) == std::string::npos) {
            continue;
        }

        if (item.first.size() < maxFillLen) {
            fillLen = maxFillLen - item.first.size();
        }

        //        auto path = std::filesystem::path{fstring};

        _list.addLine(FString{item.first} +
                          FString{std::string(fillLen, ' '), 1} + item.second,
                      std::string{item.second});
    }
}
