
#include "plugin/build.h"
#include "files/popenstream.h"

void build(IEnvironment &env) {
    env.showConsole(true);
    env.console().clear();
    env.console().push_back(std::string{"trying to build..."});

    POpenStream stream("make", true);

    for (std::string line; getline(stream, line);) {
        env.console().push_back(line);
    }

    env.console().push_back(std::string{"finished..."});
}
