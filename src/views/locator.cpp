
#include "locator.h"
#include "modes/parentmode.h"
#include "text/buffer.h"

Locator::Locator(Project &projectFiles)
    : Editor{std::make_shared<Buffer>()}, _projectFiles(projectFiles) {
    buffer().singleLine(true);
    _list.x(0);
    _list.y(1);
    _list.width(20);
    _list.height(20);
}

bool Locator::keyPress(std::shared_ptr<IScope> env) {
    auto localEnvironment = std::make_shared<Scope>(env);
    localEnvironment->editor(this);
    ;
    if (_list.keyPress(localEnvironment)) {
        return true;
    }
    else {
        auto ret = Editor::keyPress(localEnvironment);

        updateList();
        return ret;
    }
}

Locator::~Locator() = default;

void Locator::draw(IScreen &screen) {
    if (Editor::visible()) {
        Editor::draw(screen);
        _list.draw(screen);
    }
}

void Locator::callback(std::function<void(filesystem::path)> f) {
    _list.callback([f, this](auto &&, size_t, auto &&path) {
        if (path.has_value()) {
            f(std::any_cast<filesystem::path>(path));
        }
        else {
            f({});
        }
        buffer().clear();
    });
}

void Locator::visible(bool value) {
    _list.visible(value);
    Editor::visible(value);

    if (value) {
        updateList();
    }
}

void Locator::updateList() {
    const size_t maxFillLen = 20;
    _list.clear();

    auto bufferStr = buffer().text();

    for (auto &path : _projectFiles.files()) {
        size_t fillLen = 1;
        auto str = path.filename().string();

        if (str.find(bufferStr) == std::string::npos) {
            continue;
        }

        if (str.size() < maxFillLen) {
            fillLen = maxFillLen - str.size();
        }

        _list.addLine(FString{path.filename().string() +
                                  std::string(fillLen, ' ') + path.string(),
                              1},
                      path);
    }
}
