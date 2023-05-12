
#include "localenvironment.h"
#include "core/coreenvironment.h"
#include "views/mainwindow.h"
#include <stdexcept>

LocalEnvironment::LocalEnvironment(MainWindow &mw, ThreadContext &context)
    : _mainWindow{mw}
    , _context{context} {}

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

void LocalEnvironment::redrawScreen() {
    mainWindow().triggerRedraw();
}

void LocalEnvironment::statusMessage(FStringView str) {
    _mainWindow.statusMessage(str);
}
