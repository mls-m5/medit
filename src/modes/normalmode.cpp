
#include "normalmode.h"

void NormalMode::keyPress(const KeyEvent &event, Editor &) {
    //    switch (event.key) { case Key::Text: }
}

std::string_view NormalMode::name() {
    return "normal";
}
