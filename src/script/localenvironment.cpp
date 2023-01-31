
#include "localenvironment.h"
#include "core/coreenvironment.h"

CoreEnvironment &LocalEnvironment::core() {
    return CoreEnvironment::instance();
}
