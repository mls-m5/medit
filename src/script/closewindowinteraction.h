#pragma once

#include "meditfwd.h"
#include <functional>
#include <memory>

// Asks about every modified or missing disk-backed buffer, including those
// not shown in the active editor. true means the window may now close.
void beginCloseWindowInteraction(std::shared_ptr<IEnvironment>,
                                 std::function<void(bool)> completed);
