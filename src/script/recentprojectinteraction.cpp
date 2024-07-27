

#include "files/projectsettings.h"
#include "interaction.h"
#include "main.h"
#include "script/staticcommandregister.h"
#include "views/interactionhandling.h"
#include <iostream>

namespace {

void handleRecentProjectResponse(std::shared_ptr<IEnvironment> env,
                                 const Interaction &i) {
    auto project = i.lineAtCursor();
    auto settings = ProjectSettings{};

    if (!settings.load(project)) {
        std::cout << "cannot open project file " + i.lineAtCursor()
                  << std::endl;
        return;
    }

    restartMedit(env->context(), {project});
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
