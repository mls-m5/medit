
#pragma once

#include "meditfwd.h"
#include "modes/imode.h"
#include <memory>

std::shared_ptr<IMode> createVisualMode(IEnvironment &env,
                                        bool blockSelection = false);
