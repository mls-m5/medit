#include "ienvironment.h"
#include "script/staticcommandregister.h"
#include <iostream>
#include <memory>

void showSwitchFile(std::shared_ptr<IEnvironment> env) {
    std::cout << "should switch file here" << std::endl;
}

namespace {

StaticCommandRegister switchReg("switch_file", showSwitchFile);

}
