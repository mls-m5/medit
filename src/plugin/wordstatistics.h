#pragma once

#include "script/ienvironment.h"
#include "script/standardcommands.h"
#include <memory>

void beginWordStatisticsPlugin(std::shared_ptr<IEnvironment> env);

void registerWordStatistics(StandardCommands &);
