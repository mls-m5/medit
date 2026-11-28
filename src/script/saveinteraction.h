#pragma once

#include "meditfwd.h"
#include <memory>

void saveInteraction(std::shared_ptr<IEnvironment>);

/// Shows why `buffer` differs from its file on disk, in the error color.
/// Nothing when it is in sync.
void showDiskConflict(std::shared_ptr<IEnvironment>, Buffer &buffer);
