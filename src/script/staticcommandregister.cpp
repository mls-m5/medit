#include "staticcommandregister.h"
#include "standardcommands.h"

StaticCommandRegister::StaticCommandRegister(std::string name,
                                             FunctionType function) {
    StandardCommands::get().addCommand(name, function, nullptr);
}
