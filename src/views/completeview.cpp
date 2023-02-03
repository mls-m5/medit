#include "completeview.h"
#include "keys/event.h"
#include "script/iscope.h"
#include "text/cursorops.h"
#include "text/cursorrange.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"

CompleteView::CompleteView() {
    _list.width(20);
    _list.height(20);
}

void CompleteView::triggerShow(std::shared_ptr<IScope> env) {
    _list.visible(true);
    visible(true);

    _list.clear();

    _autoComplete.populate(env,
                           [env, this]() { updateCompletion(_currentText); });
}

void CompleteView::updateCompletion(std::string str) {
    constexpr bool debug = true;

    auto values = _autoComplete.getMatching(str);
    auto selected = _list.current();
    _list.clear();
    for (auto &item : values) {
        _list.addLine(FString{item.name + "     ", 0} + item.description,
                      item.edit.text);
    }

    if (debug) {
        if (_list.isEmpty()) {
            _list.addLine(str + " not found", "");
        }
    }

    _list.current(selected);
}

bool CompleteView::keyPress(std::shared_ptr<IScope> env) {
    if (env->env().key().key == Key::Text ||
        env->env().key().key == Key::Backspace ||
        env->env().key().key == Key::Delete) {
        setCursor(env->editor().cursor(), env->editor().bufferView());
    }
    return _list.keyPress(env);
}

void CompleteView::callback(
    std::function<void(CompleteView::CompletionResult)> f) {
    _list.callback([f, this](auto &&, auto &&index, auto &&value) {
        if (index != static_cast<size_t>(-1)) {
            try {
                auto str = std::any_cast<std::string>(value);
                f({str.substr(_currentText.size()), ""});
            }
            catch (std::bad_any_cast &e) {
                f({"", ""});
            }
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
    auto begin = autocompleteWordBegin(cursor);
    auto range = CursorRange{begin, cursor};
    currentText(content(range));

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
