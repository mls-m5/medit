
#include "locator.h"
#include "modes/parentmode.h"

Locator::Locator(IEnvironment &env) : _env(&env) {
    _env.editor(this);
}
