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
#include <filesystem>
#include <string>
#include <string_view>

namespace {

// Used to remove cursor when program exits
std::filesystem::path lastRunningPosition = {};

} // namespace

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

    static constexpr auto runningDebuggerName = "running-debugger-name";

    auto stateCallback = [wenv = env->weak_from_this()](DebuggerState state) {
        auto env = wenv.lock();
        if (state.state == DebuggerState::Running) {
            return;
            env->statusMessage(FString{"running application"});
        }

        if (!lastRunningPosition.empty()) {
            env->context().guiQueue().addTask([env] {
                env->core().files().publishDiagnostics(
                    lastRunningPosition, runningDebuggerName, {});
            });
        }

        if (state.state == DebuggerState::Stopped) {
            return;
            env->statusMessage(FString{"running application"});
        }

        env->context().guiQueue().addTask([state, env] {
            env->mainWindow().open(state.location.path,
                                   state.location.position.x(),
                                   state.location.position.y());

            auto d = std::vector<Diagnostics::Diagnostic>{};
            d.push_back(Diagnostics::Diagnostic{
                .type = DiagnosticType::RunningPosition,
                .source = runningDebuggerName,
                .message = "Paused position",
                .range = {.begin = state.location.position,
                          .end = state.location.position},
            });
            env->core().files().publishDiagnostics(
                state.location.path, runningDebuggerName, std::move(d));
            lastRunningPosition = state.location.path;
        });
    };

    auto breakpointCallback =
        [wenv = env->weak_from_this()](const BreakpointList &infos) {
            auto env = wenv.lock();
            /// Todo create some indicator
            env->context().guiQueue().addTask([infos = std::move(infos), env] {
                for (auto &it : infos) {
                    constexpr auto diagnosticName = "debugger";
                    auto d = std::vector<Diagnostics::Diagnostic>{};

                    for (auto &info : it.second) {
                        d.push_back({
                            .type = DiagnosticType::Breakpoint,
                            .source = diagnosticName,
                            .message = "breakpoint ",
                            .range = {.begin = {0, info.lineNumber},
                                      .end = {1, info.lineNumber}},
                        });
                    }

                    env->core().files().publishDiagnostics(
                        it.first, diagnosticName, std::move(d));
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
