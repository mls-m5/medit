#include "staticcommandregister.h"
#include "standardcommands.h"

StaticCommandRegister::StaticCommandRegister(std::string name,
                                             FunctionType function) {
    StandardCommands::get().addCommand(name, function, nullptr);
}

StaticCommandRegister::StaticCommandRegister(std::vector<Entity> entities) {
    for (auto &entity : entities) {
        StandardCommands::get().addCommand(
            entity.name, entity.function, nullptr);
    }
}
