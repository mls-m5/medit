
#pragma once

#include "modes/imode.h"
#include <memory>

std::unique_ptr<IMode> createVisualMode();
