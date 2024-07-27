#pragma once

#include "core/context.h"
#include "core/ijobqueue.h"
#include <string>
#include <vector>

namespace medit {
namespace main {

// Note this is inlined so that main.cpp can be omitted in some cases

inline bool shouldQuit = false;

inline bool shouldRestart = false;
// This is used if the program is requested to restart after quiting
inline std::vector<std::string> restartArguments;

} // namespace main
} // namespace medit

inline void quitMedit(ThreadContext &context) {
    medit::main::shouldQuit = true;
    context.guiQueue().stop();
}

inline void restartMedit(ThreadContext &context,
                         std::vector<std::string> newArguments) {
    medit::main::shouldRestart = true;
    medit::main::restartArguments = std::move(newArguments);
    quitMedit(context);
}
