#include "renameinteraction.h"
#include "core/coreenvironment.h"
#include "ienvironment.h"
#include "interaction.h"
#include "syntax/irename.h"
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

void renameVerifiedCallback(std::shared_ptr<IEnvironment> env,
                            IRename::PrepareCallbackArgs args) {
    //    auto range = CursorRange{::wordBegin(cursor),
    //    ::right(::wordEnd(cursor))}; auto old = content(range);

    auto fileStr = std::string{};

    auto &e = env->editor();

    auto &buffer = e.buffer();

    auto range = CursorRange{buffer, args.start, args.end};
    auto old = content(range);

    auto i = Interaction{"rename",
                         {
                             {"from", old},
                             {"to", old},
                             {"file", e.file()->path().string()},
                             {"l", std::to_string(args.start.y())},
                             {"c", std::to_string(args.start.x())},
                         },
                         {4, 2}};

    env->mainWindow().interactions().newInteraction(i,
                                                    handleUserRenameResponse);
}

} // namespace

void beginRenameInteraction(std::shared_ptr<IEnvironment> env) {
    auto &e = env->editor();
    //    auto cursor = e.cursor();

    auto &renamePlugins = env->core().plugins().get<IRename>();

    for (auto &rename : renamePlugins) {
        //        auto args =
        //        IRename::PrepareCallbackArgs{std::string{i.at("to")}};
        if (rename->prepare(env, [env](IRename::PrepareCallbackArgs args) {
                renameVerifiedCallback(env, std::move(args));
                //                handleRenameChanges(env,
                //                std::move(changes));
            })) {
            break;
        }
    }
}