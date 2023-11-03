#include "rundebug.h"
#include "core/coreenvironment.h"
#include "core/ijobqueue.h"
#include "plugin/idebugger.h"
#include "script/ienvironment.h"
#include "syntax/palette.h"
#include "text/diagnostics.h"
#include "text/fstring.h"
#include "views/editor.h"
#include "views/mainwindow.h"
#include <string_view>

void debug(std::shared_ptr<IEnvironment> env) {
    auto debugger = env->core().debugger();

    env->showConsole(true);

    auto print = [wenv = env->weak_from_this()](std::string_view str) {
        auto env = wenv.lock();
        env->context().guiQueue().addTask([env, str = std::string{str}] {
            env->console().buffer().pushBack(str);
            auto cursor = env->console().buffer().end();
            cursor.x(0);
            env->console().cursor(cursor);
        });
    };
    auto printDebug = [wenv = env->weak_from_this()](std::string_view str) {
        auto env = wenv.lock();
        env->context().guiQueue().addTask([env, str = std::string{str}] {
            env->console().buffer().pushBack(FString{str, Palette::comment});
            auto cursor = env->console().buffer().end();
            //            cursor.x(0);
            //            env->console().cursor(cursor);
        });
    };

    auto statusMessage = [wenv = env->weak_from_this()](std::string_view str) {
        auto env = wenv.lock();
        env->context().guiQueue().addTask([env, str = std::string{str}] {
            env->statusMessage(FString{str});
        });
    };

    auto stateCallback = [wenv = env->weak_from_this()](DebuggerState state) {
        auto env = wenv.lock();
        if (state.state == DebuggerState::Running) {
            return;
            env->statusMessage(FString{"running application"});
        }

        /// Todo create some indicator
        env->context().guiQueue().addTask([state, env] {
            env->mainWindow().open(state.location.path,
                                   state.location.position.x(),
                                   state.location.position.y());
        });
    };

    auto breakpointCallback =
        [wenv = env->weak_from_this()](const BreakpointList &infos) {
            auto env = wenv.lock();
            /// Todo create some indicator
            env->context().guiQueue().addTask([infos = std::move(infos), env] {
                for (auto &it : infos) {
                    auto d = std::vector<Diagnostics::Diagnostic>{};

                    for (auto &info : it.second) {
                        d.push_back({
                            .type = DiagnosticType::Breakpoint,
                            .source = "debugger",
                            .message = "breakpoint",
                            .range = {.begin = {0, info.lineNumber},
                                      .end = {Position::max, info.lineNumber}},
                        });
                    }

                    env->core().files().publishDiagnostics(
                        it.first, "gdb", std::move(d));
                }
            });
        };

    if (!debugger) {
        print("no debugger found");
        return;
    }

    debugger->command(env->project().settings().debug.command);
    debugger->workingDirectory(env->project().settings().debug.workingDir);
    debugger->applicationOutputCallback(print);
    debugger->debuggerOutputCallback(printDebug);
    debugger->gdbStatusCallback(statusMessage);
    debugger->stateCallback(stateCallback);
    debugger->breakpointListCallback(breakpointCallback);

    debugger->run();
}
