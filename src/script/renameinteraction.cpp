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

    if (auto f = e.file()) {
        // TODO: Handle for buffers that is not saved
        fileStr = e.file()->path().string() + ":" + std::to_string(cursor.y()) +
                  "," + std::to_string(cursor.x());
    }

    auto i = Interaction{"rename",
                         {
                             {"from", old},
                             {"to", old},
                             {"to", fileStr},
                         }};

    env->mainWindow().interactions().newInteraction(i,
                                                    handleUserRenameResponse);
}
