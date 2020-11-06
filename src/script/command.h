#pragma once

#include "script/parser.h"
#include <string>
#include <vector>

class Command {
public:
    std::string text;

    Command() = default;
    Command(const Command &) = default;
    Command(Command &&) = default;
    Command &operator=(const Command &) = default;
    Command &operator=(Command &&) = default;

    ~Command() = default;

    operator bool() const {
        return !text.empty();
    }
};

class CommandBlock {
public:
    Command _command;
    std::vector<CommandBlock> list;

    CommandBlock(std::string code);
    CommandBlock(Command command) : _command(std::move(command)) {}

    CommandBlock() = default;
    CommandBlock(const CommandBlock &) = default;
    CommandBlock(CommandBlock &&) = default;
    CommandBlock &operator=(const CommandBlock &) = default;
    CommandBlock &operator=(CommandBlock &&) = default;

    auto begin() {
        return list.begin();
    }

    auto end() {
        return list.end();
    }

    auto begin() const {
        return list.begin();
    }

    auto end() const {
        return list.end();
    }

    operator bool() const {
        return !list.empty();
    }

    ~CommandBlock() = default;
};
