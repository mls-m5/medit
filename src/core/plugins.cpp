#include "core/plugins.h"
#include "completion/icompletionsource.h"
#include "syntax/ihighlight.h"
#include <algorithm>
#include <vector>

namespace {

struct PluginData {
    std::vector<PluginRegisterFunctions::HighlightFactoryF>
        highligtFactoryFunctions;
    std::vector<PluginRegisterFunctions::CompletionFactoryF>
        completionFactoryFunctions;

    PluginData() {
        highligtFactoryFunctions.reserve(20);
        completionFactoryFunctions.reserve(20);
    }
};

// Something like a singleton
PluginData &pluginData() {
    static PluginData data;
    return data;
}

} // namespace

PluginRegisterFunctions pluginRegisterFunctions() {
    return PluginRegisterFunctions{
        .registerHighlighting =
            [](PluginRegisterFunctions::HighlightFactoryF f) {
                pluginData().highligtFactoryFunctions.push_back(std::move(f));
            },
        .registerCompletion =
            [](PluginRegisterFunctions::CompletionFactoryF f) {
                pluginData().completionFactoryFunctions.push_back(std::move(f));
            },
    };
}

std::vector<std::unique_ptr<IHighlight>> createHighlightings() {
    std::vector<std::unique_ptr<IHighlight>> ret;
    auto &pluginData = ::pluginData();
    ret.reserve(pluginData.highligtFactoryFunctions.size());

    for (auto &f : pluginData.highligtFactoryFunctions) {
        ret.push_back(f());
    }

    return ret;
}
