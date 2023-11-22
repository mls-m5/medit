#include "interaction.h"
#include "views/interactionhandling.h"

void Interaction::begin(InteractionHandling &interactions,
                        InteractionCallback callback) {
    interactions.newInteraction(*this, callback);
}
