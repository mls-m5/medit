#pragma once

#include "script/ienvironment.h"

#include <functional>

/// Runs the project's build command in the console. `then` is called on the
/// gui thread with whether the build succeeded; it is not called when a build
/// is already running.
void build(std::shared_ptr<IEnvironment> env,
           std::function<void(bool succeeded)> then = {});
