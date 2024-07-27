

#include "files/projectsettings.h"
#include "interaction.h"
#include "script/staticcommandregister.h"
#include "views/interactionhandling.h"
#include <iostream>

namespace {

void handleRecentProjectResponse(std::shared_ptr<IEnvironment> env,
                                 const Interaction &i) {
    // TODO: Implement this
    std::cout << "recent projects... to be implemented" << std::endl;
}

void beginRecentProjectInteraction(std::shared_ptr<IEnvironment> env) {
    auto i = Interaction{
        .text = "",
        .title = "open recent project...",
    };

    for (auto &project : ProjectSettings::fetchRecentProjects()) {
        i.text += project.settingsPath.string() + "\n";
    }

    i.begin(env->interactions(), handleRecentProjectResponse);
}

StaticCommandRegister recentProjectReg{"recent_projects",
                                       beginRecentProjectInteraction};

} // namespace
