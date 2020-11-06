
#include "locator.h"
#include "modes/parentmode.h"

Locator::Locator(IEnvironment &env) : _env(&env) {
    _env.editor(this);
    //    _list.addLine("hello");
    //    _list.addLine("there");
    _list.x(0);
    _list.y(1);
    _list.width(20);
    _list.height(20);
}

bool Locator::keyPress(IEnvironment &) {
    if (_list.keyPress(_env)) {
        return true;
    }
    else {
        auto ret = Editor::keyPress(_env);

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
    _list.callback([f, this](auto &&, auto &&path) {
        f(std::any_cast<filesystem::path>(path));
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

void Locator::updateCache(filesystem::path pathInProject) {
    _fileCache = _projectFiles.allProjectFiles(pathInProject);
}

void Locator::updateList() {
    const size_t maxFillLen = 20;
    _list.clear();

    auto bufferStr = buffer().text();

    for (auto &path : _fileCache) {
        size_t fillLen = 1;
        auto str = path.filename().string();

        if (str.find(bufferStr) == std::string::npos) {
            continue;
        }

        if (str.size() < maxFillLen) {
            fillLen = maxFillLen - str.size();
        }

        _list.addLine(path.filename().string() + std::string(fillLen, ' ') +
                          path.string(),
                      path);
    }
}
