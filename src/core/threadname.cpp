#include "threadname.h"
#include "os.h"
#include <array>
#include <string>

#ifdef MEDIT_USING_LINUX
#include <pthread.h>
#endif

void setThreadName(const std::string &name) {
#ifdef MEDIT_USING_LINUX
    pthread_setname_np(pthread_self(), name.c_str());
#endif
}

std::string getThreadName() {
#ifdef MEDIT_USING_LINUX
    auto nameArr = std::array<char, 1000>{};
    auto size =
        pthread_getname_np(pthread_self(), nameArr.data(), nameArr.size());
    return std::string{nameArr.data(), nameArr.data() + size};
#endif
    return {};
}
