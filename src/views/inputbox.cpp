#include "views/inputbox.h"
#include "keys/keyevent.h"
#include "modes/insertmode.h"
#include "screen/draw.h"
#include "screen/iscreen.h"
#include "script/environment.h"
#include "script/ienvironment.h"
#include "syntax/ipalette.h"
#include "text/buffer.h"

InputBox::InputBox(FString title, std::string defaultValue) : _title(title) {
    x(10);
    y(10);
    width(20);
    height(100);

    _entry.x(11);
    _entry.y(12);
    _entry.height(1);

    _entry.buffer().text(defaultValue);
    _entry.cursor(_entry.buffer().end());
    _entry.mode(createInsertMode());
}

InputBox::~InputBox() = default;

bool InputBox::keyPress(std::shared_ptr<IEnvironment> env) {
    auto key = env->key();

    auto context = std::make_shared<Environment>(env);

    context->editor(&_entry);

    switch (key.key) {
    case Key::Escape:
        env->set("return", {});
        close();
        break;
    case Key::Text:
        if (key.symbol == '\n') {
            close();
            //! To make sure that the editor points right again
            //            auto callbackContext = Environment{&env};
            //            auto callbackVariable = env->get("callback");
            //            callbackContext.set("value",
            //            {_entry.buffer().text()}); if (callbackVariable) {
            //                callbackContext.run(CommandBlock{callbackVariable->value()});
            //            }
            _callback(_entry.buffer().text());
        }
        if (_entry.keyPress(context)) {
            return true;
        }
        break;
    default:
        if (_entry.keyPress(context)) {
            return true;
        }
    }

    return true;
}

void InputBox::updateCursor(IScreen &screen) const {
    _entry.updateCursor(screen);
}

void InputBox::draw(IScreen &screen) {
    width(screen.width() - x());
    ::fillRect(screen, {' ', IPalette::standard}, *this);
    ::draw(screen, x() + 1, y() + 1, _title);

    _entry.draw(screen);
}
