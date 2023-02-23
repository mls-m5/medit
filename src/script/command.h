#pragma once

#include "ienvironment.h"
#include "meditfwd.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

// TODO: Phase out this class
class Command {
public:
    using FT = std::function<void(std::shared_ptr<IEnvironment>)>;

    std::string text;
    FT f;

    Command() = default;
    Command(const Command &) = default;
    Command(Command &&) = default;
    Command &operator=(const Command &) = default;
    Command &operator=(Command &&) = default;

    ~Command() = default;

    operator bool() const {
        return !text.empty() || f;
    }
};

// TODO: This whole class should go aswell and be replaced by std-function
class CommandBlock {
public:
    Command _command;
    std::vector<CommandBlock> list;

    CommandBlock(Command::FT f) {
        _command.f = f;
    }

    CommandBlock(std::string code);
    CommandBlock(Command command)
        : _command(std::move(command)) {}

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
        return !list.empty() || _command;
    }

    ~CommandBlock() = default;

    // This whole class will be deleted this is just to be able to use it as a
    // funuction until then
    void operator()(std::shared_ptr<IEnvironment> env) const {
        _command.f(std::move(env));
    }
};
