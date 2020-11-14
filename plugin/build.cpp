
#include "plugin/build.h"
#include "files/popenstream.h"
#include "files/project.h"
#include "views/editor.h"

void build(IEnvironment &env) {
    env.showConsole(true);
    auto &consoleBuffer = env.console().buffer();
    consoleBuffer.clear();
    consoleBuffer.push_back(std::string{"trying to build..."});

    auto &project = env.project();

    POpenStream stream(project.settings().buildCommand, true);

    for (std::string line; getline(stream, line);) {
        consoleBuffer.push_back(line);
    }

    consoleBuffer.push_back(std::string{"finished..."});
}
