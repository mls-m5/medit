#pragma once

#include "script/ienvironment.h"

/// Runs the project's run command in the console. Usually called after a
/// build, whose output is then kept.
void run(std::shared_ptr<IEnvironment> scope, bool clearConsole = true);
