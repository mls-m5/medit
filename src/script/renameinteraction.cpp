#include "renameinteraction.h"
#include "ienvironment.h"
#include "interaction.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include "views/interactionhandling.h"
#include "views/mainwindow.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

namespace {

/// When the user has selected the new name, handle what happends then
void handleUserRenameResponse(std::shared_ptr<IEnvironment> env,
                              const Interaction &i) {
    std::cout << "response " << i.at("to") << std::endl;
}

} // namespace

void beginRenameInteraction(std::shared_ptr<IEnvironment> env) {
    auto &e = env->editor();
    auto cursor = e.cursor();
    auto range = CursorRange{::wordBegin(cursor), ::right(::wordEnd(cursor))};
    auto old = content(range);

    auto fileStr = std::string{};

    if (!e.file()) {
        return;
    }

    auto i = Interaction{"rename",
                         {
                             {"from", old},
                             {"to", old},
                             {"file", e.file()->path().string()},
                             {"l", std::to_string(cursor.y() + 1)},
                             {"c", std::to_string(cursor.x() + 1)},
                         },
                         {4, 2}};

    env->mainWindow().interactions().newInteraction(i,
                                                    handleUserRenameResponse);
}
