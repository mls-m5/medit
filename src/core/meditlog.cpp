#include "meditlog.h"
#include <queue>
#include <string>

namespace {

class MeditLog {
public:
    void log(std::string text) {
        loggedText.push(std::move(text));
    }

    static MeditLog &instance() {
        static MeditLog log;
        return log;
    }

    std::queue<std::string> loggedText;
};

} // namespace

void impl::logInternal(std::string text) {
    MeditLog::instance().log(std::move(text));
}
