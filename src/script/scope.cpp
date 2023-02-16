#include "scope.h"
#include "script/standardcommands.h"
#include "views/editor.h"

Scope::Scope(std::shared_ptr<IScope> parent)
    : _parent(parent) {
    addStandardCommands(*this);
    initLua(_lua);
}

void Scope::editor(Editor *editor) {
    _editor = editor;
    _lua["editor"] = editor;
}
