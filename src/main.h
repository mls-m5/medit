#pragma once

#include "core/context.h"
#include "core/ijobqueue.h"

namespace medit {
namespace main {

inline bool shouldQuit = false;
} // namespace main
} // namespace medit

inline void quitMedit(ThreadContext &context) {
    medit::main::shouldQuit = true;
    context.guiQueue().stop();
}
