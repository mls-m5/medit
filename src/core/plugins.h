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
    template <typename T>
    struct Func {
        using createT = std::unique_ptr<T> (*)();
        using registerT = void (*)(createT);
    };

    Func<IHighlight>::registerT registerHighlighting;
    Func<ICompletionSource>::registerT registerCompletion;
    Func<IAnnotation>::registerT registerAnnotation;
    Func<IFormat>::registerT registerFormat;
};

// Get a handles to the register functions
PluginRegisterFunctions pluginRegisterFunctions();

// Functions used by the application to create the plugins
std::vector<std::unique_ptr<IHighlight>> createHighlightings();
std::vector<std::unique_ptr<ICompletionSource>> createCompletionSources();
std::vector<std::unique_ptr<IAnnotation>> createAnnotations();
std::vector<std::unique_ptr<IFormat>> createFormat();

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

template <typename T>
void registerAnnotation() {
    pluginRegisterFunctions().registerAnnotation(
        []() -> std::unique_ptr<IAnnotation> { return std::make_unique<T>(); });
}

template <typename T>
void registerFormat() {
    pluginRegisterFunctions().registerFormat(
        []() -> std::unique_ptr<IFormat> { return std::make_unique<T>(); });
}
