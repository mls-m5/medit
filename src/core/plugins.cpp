#include "core/plugins.h"
#include "completion/icompletionsource.h"
#include "navigation/inavigation.h"
#include "syntax/iannotation.h"
#include "syntax/iformat.h"
#include "syntax/ihighlight.h"
#include <algorithm>
#include <vector>

namespace {

struct PluginData {
    std::vector<PluginRegisterFunctions::Func<IHighlight>::createT>
        highligtCreateFunctions;
    std::vector<PluginRegisterFunctions::Func<ICompletionSource>::createT>
        completionCreateFunctions;
    std::vector<PluginRegisterFunctions::Func<IAnnotation>::createT>
        annotationCreateFunctions;
    std::vector<PluginRegisterFunctions::Func<IFormat>::createT>
        formatCreateFunctions;
    std::vector<PluginRegisterFunctions::Func<INavigation>::createT>
        navigationCreateFunctions;
};

// Something like a singleton
PluginData &pluginData() {
    static PluginData data;
    return data;
}

// Generic create function
template <typename Type, typename FactoryFunctions>
std::vector<std::unique_ptr<Type>> createPlugins(FactoryFunctions &functions) {
    std::vector<std::unique_ptr<Type>> ret;
    ret.reserve(functions.size());

    for (auto &f : functions) {
        ret.push_back(f());
    }

    return ret;
}

} // namespace

PluginRegisterFunctions pluginRegisterFunctions() {
    return PluginRegisterFunctions{
        .registerHighlighting =
            [](PluginRegisterFunctions::Func<IHighlight>::createT f) {
                pluginData().highligtCreateFunctions.push_back(std::move(f));
            },
        .registerCompletion =
            [](PluginRegisterFunctions::Func<ICompletionSource>::createT f) {
                pluginData().completionCreateFunctions.push_back(std::move(f));
            },
        .registerAnnotation =
            [](PluginRegisterFunctions::Func<IAnnotation>::createT f) {
                pluginData().annotationCreateFunctions.push_back(std::move(f));
            },
        .registerFormat =
            [](PluginRegisterFunctions::Func<IFormat>::createT f) {
                pluginData().formatCreateFunctions.push_back(std::move(f));
            },
        .registerNavigation =
            [](PluginRegisterFunctions::Func<INavigation>::createT f) {
                pluginData().navigationCreateFunctions.push_back(std::move(f));
            },
    };
}

std::vector<std::unique_ptr<IHighlight>> createHighlightings() {
    return createPlugins<IHighlight>(pluginData().highligtCreateFunctions);
}

std::vector<std::unique_ptr<ICompletionSource>> createCompletionSources() {
    return createPlugins<ICompletionSource>(
        pluginData().completionCreateFunctions);
}

std::vector<std::unique_ptr<IAnnotation>> createAnnotations() {
    return createPlugins<IAnnotation>(pluginData().annotationCreateFunctions);
}

std::vector<std::unique_ptr<IFormat>> createFormat() {
    return createPlugins<IFormat>(pluginData().formatCreateFunctions);
}

std::vector<std::unique_ptr<INavigation>> createNavigation() {
    return createPlugins<INavigation>(pluginData().navigationCreateFunctions);
}
