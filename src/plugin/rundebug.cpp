#include "rundebug.h"
#include "core/coreenvironment.h"
#include "core/ijobqueue.h"
#include "script/ienvironment.h"
#include "views/editor.h"
#include <string_view>

void debug(std::shared_ptr<IEnvironment> env) {
    auto debugger = env->core().debugger();

    env->showConsole(true);

    auto print = [wenv = env->weak_from_this()](std::string_view str) {
        auto env = wenv.lock();
        env->context().guiQueue().addTask([env, str = std::string{str}] {
            env->console().buffer().pushBack(str);
            env->console().cursor(env->console().buffer().end());
        });
    };

    if (!debugger) {
        print("no debugger found");
        return;
    }

    debugger->applicationOutputCallback(print);

    debugger->run();
}
