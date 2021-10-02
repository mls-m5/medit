#include "headernavigation.h"
#include "core/plugins.h"
#include "files/extensions.h"
#include "files/file.h"
#include "files/project.h"
#include "script/environment.h"
#include "script/ienvironment.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include <optional>

namespace {

std::optional<std::string> getIncludeName(IEnvironment &env) {
    auto cursor = env.editor().cursor();
    auto line = ::line(cursor);
    {
        auto content = ::content(line);
        if (content.empty()) {
            return {};
        }
    }

    auto firstQuotation = find(cursor, {'"'});
    if (!firstQuotation) {
        return {};
    }

    auto firstQuotCur = right(*firstQuotation, false);

    auto range = inner(firstQuotCur, '\"', '\"');

    if (range.empty()) {
        return {};
    }

    auto includeName = ::content(range);
    if (includeName.empty()) {
        return {};
    }

    return includeName.front();
}

bool openIncludeByName(std::shared_ptr<IEnvironment> env, std::string name) {
    auto files = env->project().files();

    for (auto &path : files) {
        if (path.string().find(name) != std::string::npos) {
            auto localEnvironment = std::make_shared<Environment>(env);
            localEnvironment->set("path", path.string());
            localEnvironment->run(Command{"editor.open"});
            return true;
        }
    }

    return false;
}

} // namespace

bool HeaderNavigation::gotoSymbol(std::shared_ptr<IEnvironment> env) {

    auto file = env->editor().file();
    if (!file) {
        return false;
    }
    auto filename = file->path();

    if (!isCpp(filename)) {
        return false;
    }

    if (auto includeName = getIncludeName(*env)) {
        return openIncludeByName(env, *includeName);
    }

    return false;
}

void HeaderNavigation::registerPlugin() {
    registerNavigation<HeaderNavigation>();
}
