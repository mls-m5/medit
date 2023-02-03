#pragma once

#include "keys/event.h"
#include "modes/imode.h"

#include <memory>

std::shared_ptr<IMode> createInsertMode();
