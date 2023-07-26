#include "threadname.h"
#include "os.h"

#ifdef MEDIT_USING_LINUX
#include <pthread.h>
#endif

void setThreadName(const std::string &name) {
#ifdef MEDIT_USING_LINUX
    pthread_setname_np(pthread_self(), name.c_str());
#endif
}
