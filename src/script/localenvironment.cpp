
#include "localenvironment.h"
#include "core/coreenvironment.h"
#include "script/initlua.h"
#include "views/mainwindow.h"

LocalEnvironment::LocalEnvironment(MainWindow &mw, Context &context)
    : _mainWindow{mw}
    , _context{context} {}

void LocalEnvironment::initLua(MainWindow &mw) {
    ::initLua(_lua, mw);
}

CoreEnvironment &LocalEnvironment::core() {
    return CoreEnvironment::instance();
}

void LocalEnvironment::parseLua(std::string_view code) {
    try {
        auto res = _lua.load(code);
        res();
    }
    catch (sol::error &e) {
        std::cerr << e.what() << "\n";
    }
}

void LocalEnvironment::redrawScreen() {
    mainWindow().triggerRedraw();
}
