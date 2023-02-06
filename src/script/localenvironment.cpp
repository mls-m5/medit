
#include "localenvironment.h"
#include "core/coreenvironment.h"
#include "views/mainwindow.h"

CoreEnvironment &LocalEnvironment::core() {
    return CoreEnvironment::instance();
}

void LocalEnvironment::redrawScreen() {
    mainWindow().triggerRedraw();
}
