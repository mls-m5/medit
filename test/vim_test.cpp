/*
 * Scripted tests. The tests are loaded from a file
 * */
#include "keys/event.h"
#include "mls-unit-test/unittest.h"
#include "mock/script/mockenvironment.h"
#include "modes/normalmode.h"
#include "script/ienvironment.h"
#include "text/buffer.h"
#include "text/position.h"
#include "views/editor.h"
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct VimCodePart : public std::string {
    std::string mode;
    std::string command;
    Position position;

    void finalize() {
        if (empty()) {
            return;
        }
        if (back() == '\n') {
            pop_back();
        }

        // TODO: Handle mode, position etc
    }
};

struct VimSubTest : public std::vector<VimCodePart> {};

struct VimTest {
    std::string name;
    std::vector<VimSubTest> tests;
};

/// Split file up in sub-tests
std::vector<VimTest> loadVimTests() {
    auto file = std::ifstream{"test/res/vimtest.md"};

    bool isCode = false;

    auto ret = std::vector<VimTest>{};

    for (std::string line; std::getline(file, line);) {
        if (line.starts_with("##")) {
            while (!line.empty() &&
                   (line.front() == '#' || line.front() == ' ')) {
                line.erase(0, 1);
            }
            ret.emplace_back().name = line;
            continue;
        }

        if (line.starts_with("```")) {
            if (!isCode) {
                ret.back().tests.emplace_back().emplace_back();
            }

            isCode = !isCode;
            continue;
        }

        if (line.starts_with("===") || line.starts_with("---")) {
            ret.back().tests.back().emplace_back();
            continue;
        }

        if (isCode) {
            auto &currentTestPart = ret.back().tests.back().back();
            currentTestPart += line + "\n";
        }
    }

    // We have added extra lines. Lets remove them
    for (auto &test : ret) {
        for (auto &subTest : test.tests) {
            for (auto &part : subTest) {
                part.finalize();
            }
        }
    }

    return ret;
}

std::shared_ptr<MockEnvironment> createMockEnvironment() {
    auto env = std::make_unique<MockEnvironment>();
    createNormalMode();
    return env;
}

void emulateKeyPress(MockEnvironment &env, std::shared_ptr<std::string> keys) {
    env.mock_key_0.onCall([keys]() {
        if (keys->empty()) {
            throw std::runtime_error{"out of keys"};
        }
        else {
            auto c = keys->front();
            keys->erase(0, 1);
            return KeyEvent{Key::Text,
                            c}; // Dummy event, this should not happend
        }
    });
}

struct TestSuitVim : public unittest::StaticTestSuit {
    TestSuitVim() {
        this->testSuitName = "Vim";
        unittest::testResult = &this->testResult;

        for (auto &test : loadVimTests()) {
            newTest(test.name) = [tests = test.tests]() {
                for (auto &test : tests) {
                    auto env = createMockEnvironment();

                    auto buffer = std::make_shared<Buffer>(test.front());
                    auto editor = Editor{nullptr, buffer};

                    env->mock_editor_0.returnValueRef(editor);

                    std::cout << "should run:\n";

                    for (auto &part : test) {
                        auto keys = std::make_shared<std::string>(part.command);
                        emulateKeyPress(*env, keys);
                        editor.keyPress(env);
                        std::cout << "part:" << part << "\n";
                    }
                    std::cout.flush();
                }
            };
        }
    }

} TestSuitVim__instance;
