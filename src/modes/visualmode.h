
#pragma once

#include "imode.h"
#include <memory>

std::shared_ptr<IMode> createVisualMode(bool blockSelection = false);
