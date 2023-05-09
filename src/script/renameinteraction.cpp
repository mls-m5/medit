#include "renameinteraction.h"
#include "core/coreenvironment.h"
#include "ienvironment.h"
#include "interaction.h"
#include "syntax/irename.h"
#include "text/changes.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include "views/interactionhandling.h"
#include "views/mainwindow.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

namespace {

void applyRenameChanges(std::shared_ptr<IEnvironment> env,
                        const Interaction &i) {

    auto changes = Changes{};

    auto ss = std::istringstream{i.text};
    changes.deserialize(ss);

    changes.sort();

    changes.apply(*env);

    //    // TODO: Apply changes
    //    std::cout << "got response from rename lsp" << std::endl;
    //    changes.serialize(std::cout);
    //    std::cout.flush();
}

void lspRenameResponse(std::shared_ptr<IEnvironment> env, Changes changes) {

    auto i = Interaction{"apply rename"};

    if (changes.changes.empty()) {
        return;
    }

    auto ss = std::ostringstream{};
    changes.serialize(ss);

    i.text = ss.str();
    env->mainWindow().interactions().newInteraction(i, applyRenameChanges);
}

/// When the user has selected the new name, handle what happends then
void handleUserRenameResponse(std::shared_ptr<IEnvironment> env,
                              const Interaction &i) {
    //    std::cout << "user input " << i.at("to") << std::endl;

    auto &renamePlugins = env->core().plugins().get<IRename>();

    for (auto &rename : renamePlugins) {
        auto si = SimpleInteraction{};
        si.deserialize(i.text);

        auto args = IRename::RenameArgs{std::string{si.at("to")}};
        if (rename->rename(env, args, [env](Changes changes) {
                lspRenameResponse(env, std::move(changes));
            })) {
            break;
        }
    }
}

void renameVerifiedCallback(std::shared_ptr<IEnvironment> env,
                            IRename::PrepareCallbackArgs args) {
    auto fileStr = std::string{};

    auto &e = env->editor();

    auto &buffer = e.buffer();

    auto range = CursorRange{buffer, args.start, args.end};
    auto old = content(range);

    auto si = SimpleInteraction{"rename",
                                {
                                    {"from", old},
                                    {"to", old},
                                    {"file", e.file()->path().string()},
                                    {"l", std::to_string(args.start.y())},
                                    {"c", std::to_string(args.start.x())},
                                }};

    auto i = Interaction{
        si.serialize(),
        {4, 2},
    };

    env->mainWindow().interactions().newInteraction(i,
                                                    handleUserRenameResponse);
}

} // namespace

void beginRenameInteraction(std::shared_ptr<IEnvironment> env) {
    auto &e = env->editor();
    auto &renamePlugins = env->core().plugins().get<IRename>();

    for (auto &rename : renamePlugins) {
        if (rename->prepare(env, [env](IRename::PrepareCallbackArgs args) {
                renameVerifiedCallback(env, std::move(args));
            })) {
            break;
        }
    }
}
