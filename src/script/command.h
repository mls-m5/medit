#pragma once

#include "meditfwd.h"
#include <functional>
#include <memory>

using CommandT = std::function<void(std::shared_ptr<IEnvironment>)>;
