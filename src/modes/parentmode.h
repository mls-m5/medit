#pragma once

#include "imode.h"
#include <memory>

//! A mode that is shared between other modes
std::unique_ptr<IMode> createParentMode();
