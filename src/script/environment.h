#pragma once

#include "ienvironment.h"
#include "script/command.h"
#include "script/standardcommands.h"
#include <functional>
#include <map>

class Environment : public IEnvironment {
    using Action = std::function<void(IEnvironment &)>;
    using Functions = std::map<std::string, Action>;

    IEnvironment *_parent = nullptr;
    Editor *_editor = nullptr;

    Functions _context;

    std::map<std::string, Variable> _variables;

public:
    Environment(IEnvironment *parent) : _parent(parent) {
        addStandardCommands(*this);
    }

    // Used by handler
    void editor(Editor *editor) {
        _editor = editor;
    }

    // @see IEnvironment
    IEnvironment &root() override {
        return _parent->root();
    }

    // @see IEnvironment
    const IEnvironment &root() const override {
        return _parent->root();
    }

    // @see IEnvironment
    Editor &editor() override {
        if (!_editor) {
            if (!_parent) {
                throw std::runtime_error("trying to use editor when not set");
            }
            else {
                return _parent->editor();
            }
        }
        return *_editor;
    }

    // @see IEnvironment
    Editor &console() override {
        return root().console();
    }

    // @see IEnvironment
    KeyEvent key() const override {
        return root().key();
    }

    // @see IEnvironment
    IEnvironment &parent() override {
        if (!_parent) {
            throw std::runtime_error(
                "trying to access unset parent in enviroment");
        }
        return *_parent;
    }

    Project &project() override {
        return root().project();
    }

    Context &context() override {
        return root().context();
    }

    // @see IEnvironment
    void showConsole(bool shown) override {
        root().showConsole(shown);
    }

    void addCommand(std::string name,
                    std::function<void(IEnvironment &)> f) override {
        _context[name] = f;
    }

    // @see IEnvironment
    bool run(const Command &command) override {
        auto action = findAction(command.text);
        if (action) {
            (*action)(*this);
            return true;
        }
        else {
            return false;
        }
    }

    Action *findAction(const std::string &name) override {
        auto action = _context.find(name);
        if (action != _context.end()) {
            return &action->second;
        }
        if (_parent) {
            return parent().findAction(name);
        }
        return nullptr;
    }

    Registers &registers() override {
        return root().registers();
    }

    //! @see IEnvironment
    void set(std::string name, Variable variable) override {
        _variables[name] = std::move(variable);
    }

    //! @see IEnvironment
    std::optional<Variable> get(std::string name) const override {
        if (auto f = _variables.find(name); f != _variables.end()) {
            return {f->second};
        }
        else {
            if (_parent) {
                return _parent->get(std::move(name));
            }
            return {};
        }
    }
};
