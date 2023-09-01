#pragma once

#include "meditfwd.h"
#include <memory>

/// Indent lines by amount specified in project settings
void indent(std::shared_ptr<IEnvironment> env);

/// Same as indent but the opposit
void deindent(std::shared_ptr<IEnvironment> env);
