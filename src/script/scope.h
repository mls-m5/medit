#pragma once

#include "ienvironment.h"
#include "iscope.h"
#include "script/command.h"
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>

class Scope : public IScope {
    using Action = std::function<void(std::shared_ptr<IScope>)>;
    using Functions = std::map<std::string, Action>;

    std::shared_ptr<IScope> _parent = {};
    Editor *_editor = nullptr;

    Functions _context;

    std::map<std::string, Variable> _variables;

public:
    Scope(std::shared_ptr<IScope> parent);

    // Used by handler
    // TODO: Use environment editor instead
    [[deprecated]] void editor(Editor *editor);

    // @see IEnvironment
    IScope &root() override {
        return _parent->root();
    }

    // @see IEnvironment
    const IScope &root() const override {
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

    IEnvironment &env() override {
        return root().env();
    }

    // @see IScope
    IScope &parent() override {
        if (!_parent) {
            throw std::runtime_error(
                "trying to access unset parent in enviroment");
        }
        return *_parent;
    }

    void addCommand(std::string name,
                    std::function<void(std::shared_ptr<IScope>)> f) override {
        _context[name] = f;
    }

    // @see IScope
    bool run(const Command &command) override {
        if (command.f) {
            command.f(env().shared_from_this());
            return true;
        }
        auto action = findAction(command.text);
        if (action) {
            (*action)(shared_from_this());
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
