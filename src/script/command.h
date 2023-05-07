#pragma once

#include "meditfwd.h"
#include <functional>
#include <memory>

using CommandBlock = std::function<void(std::shared_ptr<IEnvironment>)>;
