#pragma once

namespace medit {
namespace main {

inline bool shouldQuit = false;
} // namespace main
} // namespace medit

void quitMedit() {
    medit::main::shouldQuit = true;
}
