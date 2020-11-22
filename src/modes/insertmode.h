#pragma once

#include "keys/keyevent.h"
#include "modes/imode.h"

#include <memory>

std::shared_ptr<IMode> createInsertMode();
