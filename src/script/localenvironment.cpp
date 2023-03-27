
#include "localenvironment.h"
#include "core/coreenvironment.h"
// #include "script/luastate.h"
#include "views/mainwindow.h"
#include <stdexcept>

LocalEnvironment::LocalEnvironment(MainWindow &mw, Context &context)
    : _mainWindow{mw}
    , _context{context} {}

void LocalEnvironment::initLua(MainWindow &mw) {
    //    _lua.init(mw);
}

CoreEnvironment &LocalEnvironment::core() {
    return CoreEnvironment::instance();
}

const StandardCommands &LocalEnvironment::standardCommands() const {
    return StandardCommands::get();
}

Editor &LocalEnvironment::editor() {
    auto e = _mainWindow.currentEditor();
    if (e) {
        return *e;
    }

    throw std::runtime_error{"no active editor in environment"};
}

// void LocalEnvironment::parseLua(std::string_view code) {
//     try {
//         auto res = _lua.load(code);
//         res();
//     }
//     catch (sol::error &e) {
//         std::cerr << e.what() << "\n";
//     }
// }

void LocalEnvironment::redrawScreen() {
    mainWindow().triggerRedraw();
}
