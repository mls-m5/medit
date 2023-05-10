#include "coreenvironment.h"

CoreEnvironment *CoreEnvironment::_instance = nullptr;

CoreEnvironment::CoreEnvironment(ThreadContext &context)
    : _context{&context} {
    _instance = this;
}
CoreEnvironment::~CoreEnvironment() = default;

CoreEnvironment &CoreEnvironment::instance() {
    return *_instance;
}
