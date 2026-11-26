#include "lsp/connection.h"
#include "mls-unit-test/unittest.h"
#include <chrono>
#include <condition_variable>
#include <mutex>

#ifdef MEDIT_USING_LINUX
TEST_SUIT_BEGIN(LspStderr)

TEST_CASE("server stderr is delivered separately from protocol stdout") {
    using namespace std::chrono_literals;
    std::mutex mutex;
    std::condition_variable ready;
    std::string protocol;
    std::string error;
    {
        lsp::Connection connection{
            "printf 'protocol-line\\n'; printf 'error-line\\n' >&2; read input",
            [&](std::istream &input) {
                std::string line;
                while (std::getline(input, line)) {
                    std::lock_guard lock{mutex};
                    protocol += line;
                    ready.notify_all();
                }
            },
            [] {},
            [&](std::string line) {
                std::lock_guard lock{mutex};
                error += line;
                ready.notify_all();
            }};
        bool received;
        {
            std::unique_lock lock{mutex};
            received = ready.wait_for(lock, 4s, [&] {
                return protocol == "protocol-line" && error == "error-line";
            });
        }
        connection.send("exit");
        ASSERT(received, "Expected protocol and stderr from fake server");
    }
    ASSERT(protocol == "protocol-line", protocol);
    ASSERT(error == "error-line", error);
}

TEST_CASE("missing server reports stderr and finishes") {
    using namespace std::chrono_literals;
    std::mutex mutex;
    std::condition_variable ready;
    std::string error;
    bool exited = false;
    bool received = false;
    {
        lsp::Connection connection{"medit-nonexistent-language-server-for-test",
                                   [](std::istream &input) {
                                       for (std::string line;
                                            std::getline(input, line);) {
                                       }
                                   },
                                   [&] {
                                       std::lock_guard lock{mutex};
                                       exited = true;
                                       ready.notify_all();
                                   },
                                   [&](std::string line) {
                                       std::lock_guard lock{mutex};
                                       error += line;
                                       ready.notify_all();
                                   }};
        std::unique_lock lock{mutex};
        received =
            ready.wait_for(lock, 4s, [&] { return exited && !error.empty(); });
    }
    ASSERT(received, "Expected missing-server error and exit notification");
}

TEST_SUIT_END
#endif
