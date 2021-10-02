#include "headernavigation.h"
#include "core/plugins.h"
#include "files/extensions.h"
#include "files/file.h"
#include "files/project.h"
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

bool openIncludeByName(IEnvironment &env, std::string name) {
    auto files = env.project().files();

    for (auto &path : files) {
        if (path.string().find(name) != std::string::npos) {
            env.editor().file(std::make_unique<File>(path));
            env.editor().load();
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

        //        env->showConsole(true);
        //        env->console().buffer().pushBack("trying to find:");
        //        env->console().buffer().pushBack(*includeName);
        //        env->console().buffer().pushBack("");
        //        env->console().cursor(env->console().buffer().end());

        return openIncludeByName(*env, *includeName);
    }

    return false;
}

void HeaderNavigation::registerPlugin() {
    registerNavigation<HeaderNavigation>();
}
