#include "renameinteraction.h"
#include "core/coreenvironment.h"
#include "ienvironment.h"
#include "interaction.h"
#include "syntax/irename.h"
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

void handleRenameChanges(std::shared_ptr<IEnvironment> env, Changes changes) {
    std::cout << "got response from rename lsp" << std::endl;
}

/// When the user has selected the new name, handle what happends then
void handleUserRenameResponse(std::shared_ptr<IEnvironment> env,
                              const Interaction &i) {
    std::cout << "user input " << i.at("to") << std::endl;

    auto &renamePlugins = env->core().plugins().get<IRename>();

    for (auto &rename : renamePlugins) {
        auto args = IRename::RenameArgs{std::string{i.at("to")}};
        if (rename->rename(env, args, [env](Changes changes) {
                handleRenameChanges(env, std::move(changes));
            })) {
            break;
        }
    }
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
