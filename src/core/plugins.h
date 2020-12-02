#pragma once

#include <meditfwd.h>
#include <memory>
#include <vector>

//! The register functions is used by the plugin to register a function that can
//! be used to create a unique_ptr of the plugin
//!
//! The factory functions is the type of function that the register function
//! expects
//!
//! When a plugin is loaded dynamically it is supposed to receive a pointer to a
//! PluginRegisterFunctions struct and use any of the function to add itself to
//! the main application
class PluginRegisterFunctions {
public:
    using HighlightFactoryF = std::unique_ptr<IHighlight> (*)();
    using HighlightRegisterF = void (*)(HighlightFactoryF);

    HighlightRegisterF registerHighlighting;

    using CompletionFactoryF = std::unique_ptr<ICompletionSource> (*)();
    using CompletionRegisterF = void (*)(CompletionFactoryF);

    CompletionRegisterF registerCompletion;
};

// Get a handles to the register functions
PluginRegisterFunctions pluginRegisterFunctions();

// Functions used by the application to create the plugins
std::vector<std::unique_ptr<IHighlight>> createHighlightings();
std::vector<std::unique_ptr<ICompletionSource>> createCompletionSources();

template <typename T>
void registerHighlighting() {
    pluginRegisterFunctions().registerHighlighting(
        []() -> std::unique_ptr<IHighlight> { return std::make_unique<T>(); });
}

template <typename T>
void registerCompletion() {
    pluginRegisterFunctions().registerCompletion(
        []() -> std::unique_ptr<ICompletionSource> {
            return std::make_unique<T>();
        });
}
