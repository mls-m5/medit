
#include "script/parser.h"
#include "script/command.h"
#include <sstream>

CommandBlock parse(std::string code) {
    //! Todo: Improve parser in future
    CommandBlock commands;
    auto ss = std::istringstream{std::move(code)};
    for (std::string line; getline(ss, line);) {
        Command cmd;
        cmd.text = std::move(line);
        commands.list.emplace_back(std::move(cmd));
    }
    return commands;
}
