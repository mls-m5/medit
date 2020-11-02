#pragma once

#include "keys/keyevent.h"
#include "modes/imode.h"

#include <memory>

std::unique_ptr<IMode> createInsertMode();
