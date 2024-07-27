#include "interaction.h"
#include "views/interactionhandling.h"

void Interaction::begin(InteractionHandling &interactions,
                        InteractionCallback callback) {
    interactions.newInteraction(*this, callback);
}

std::string Interaction::lineAt(size_t targetNum) const {
    auto ss = std::istringstream{text};
    auto lineNum = std::size_t{};
    auto result = std::string{};
    for (std::string line; std::getline(ss, line); ++lineNum) {
        if (lineNum == targetNum) {
            result = line;
            break;
        }
    }
    return result;
}
