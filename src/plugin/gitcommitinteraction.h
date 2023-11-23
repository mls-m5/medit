#pragma once

#include "script/ienvironment.h"
#include <memory>

void beginGitCommit(std::shared_ptr<IEnvironment> env);

void beginGitCommitAmmend(std::shared_ptr<IEnvironment> env);
