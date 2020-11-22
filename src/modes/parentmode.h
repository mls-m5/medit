#pragma once

#include "imode.h"
#include <memory>

//! A mode that is shared between other modes
std::shared_ptr<IMode> createParentMode();
