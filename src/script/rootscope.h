#pragma once

// #include "ienvironment.h"
// #include "scope.h"

// class RootScope : public Scope {
//     IEnvironment &_environment;

// public:
//     RootScope(IEnvironment &env) : Scope{nullptr}, _environment{env} {}

//    IScope &root() override {
//        return *this;
//    }

//    IEnvironment &env() override {
//        return _environment;
//    }

//    const IScope &root() const override {
//        return *this;
//    }

//    IScope &parent() override {
//        throw std::runtime_error("trying to access parent of root
//        environment");
//    }
//};
