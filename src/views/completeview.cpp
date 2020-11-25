#include "completeview.h"
#include "keys/keyevent.h"
#include "script/ienvironment.h"
#include "text/cursorops.h"
#include "text/cursorrange.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"

CompleteView::CompleteView() {
    _list.width(20);
    _list.height(20);
}

void CompleteView::triggerShow(std::shared_ptr<IEnvironment> env) {
    _list.visible(true);
    visible(true);

    _autoComplete.populate(env);
    updateCompletion(_currentText);
}

void CompleteView::updateCompletion(std::string str) {
    constexpr bool debug = true;

    auto values = _autoComplete.getMatching(str);
    auto selected = _list.current();
    _list.clear();
    for (auto &item : values) {
        _list.addLine(FString{item.name + "     ", 0} + item.description,
                      item.name);
    }

    if (debug) {
        if (_list.isEmpty()) {
            _list.addLine(str + " not found", "");
        }
    }

    _list.current(selected);
}

bool CompleteView::keyPress(std::shared_ptr<IEnvironment> env) {
    if (env->key().key == Key::Text || env->key().key == Key::Backspace ||
        env->key().key == Key::Delete) {
        setCursor(env->editor().cursor(), env->editor().bufferView());
    }
    return _list.keyPress(env);
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

void CompleteView::setCursor(const Cursor cursor,
                             const BufferView &bufferView) {
    //    auto currentChar = content(left(cursor)).at(0);
    //    Cursor begin = cursor;
    //    if (isalnum(currentChar)) {
    //        // If on for example a newline
    //        begin = wordBegin(cursor);
    //    }
    //    else {
    //        begin = cursor; // I.e. Empty string
    //    }
    auto begin = autocompleteWordBegin(cursor);
    auto range = CursorRange{begin, cursor};
    currentText(content(range).front());

    auto screenPosition = bufferView.cursorPosition(begin);

    _list.x(screenPosition.x());
    _list.y(screenPosition.y() + 1);
}

void CompleteView::currentText(std::string str) {
    _currentText = std::move(str);
    if (visible()) {
        updateCompletion(_currentText);
    }
}
