#pragma once

#include "meditfwd.h"
#include <functional>
#include <memory>

using SaveInteractionCallback = std::function<void(bool)>;

void saveInteraction(std::shared_ptr<IEnvironment>,
                     SaveInteractionCallback callback = {});
