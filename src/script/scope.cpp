#include "scope.h"
#include "views/editor.h"

Scope::Scope(std::shared_ptr<IScope> parent)
    : _parent(parent) {
    //    addStandardCommands(*this);
}

void Scope::editor(Editor *editor) {
    _editor = editor;
}
