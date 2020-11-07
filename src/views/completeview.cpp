#include "completeview.h"

CompleteView::CompleteView() {
    _list.width(20);
    _list.height(20);
}

void CompleteView::triggerShow(Position cursorPosition) {
    //        _cursorPosition = bufferView.cursorPosition(cursor);
    _cursorPosition = cursorPosition;
    _list.visible(true);
    visible(true);

    //! Make this position next to cursor
    _list.x(cursorPosition.x());
    _list.y(cursorPosition.y() + 1);
    _list.current(0);

    updateCompletion(_currentText);
}

void CompleteView::updateCompletion(std::string str) {
    auto values = _autoComplete.getMatching(str);
    auto selected = _list.current();
    _list.clear();
    for (auto &item : values) {
        _list.addLine(item.name, item.name);
    }
    _list.current(selected);
}

void CompleteView::callback(
    std::function<void(CompleteView::CompletionResult)> f) {
    _list.callback([f, this](auto &&, auto &&index, auto &&value) {
        if (index != static_cast<size_t>(-1)) {
            auto str = std::any_cast<std::string>(value);
            f({str.substr(_currentText.size()), ""});
        }
        else {
            f({"", ""});
        }
        visible(false);
        _list.visible(false);
    });
}
