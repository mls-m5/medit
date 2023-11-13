/*
 * Scripted tests. The tests are loaded from a file
 * */
#include "core/registers.h"
#include "keys/event.h"
#include "mls-unit-test/unittest.h"
#include "mock/script/mockenvironment.h"
#include "modes/normalmode.h"
#include "script/ienvironment.h"
#include "script/standardcommands.h"
#include "text/buffer.h"
#include "text/cursorrangeops.h"
#include "text/position.h"
#include "views/editor.h"
#include <fstream>
#include <iterator>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

static const auto vimPartCommandRegex = std::regex{R"(\[([A-Z])(\-(.*))?\])"};
// static const auto vimPartCommandRegex = std::regex{R"(N)"};

} // namespace

struct VimCodePart : public std::string {
    std::string input;
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
        input = *this;

        auto match = std::smatch{};
        if (std::regex_search(*this, match, vimPartCommandRegex)) {
            //            std::cout << "match " << match.str() << std::endl;

            command = match[3];
            mode = match[1];

            // TODO: Handle newlines
            position.x(match.position());

            //            erase(match[0].first, match[0].second);
            erase(match.position(), match[0].str().size());
        }
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

// struct Environment: public MockEnvironment {
//     using MockEnvironment::MockEnvironment;

//    std::
//};

std::shared_ptr<MockEnvironment> createMockEnvironment() {

    auto env = std::make_unique<MockEnvironment>();

    env->mock_standardCommands_0.returnValueRef(StandardCommands::get());
    env->mock_statusMessage_1.onCall(
        [](auto msg) { std::cout << msg << "\n"; });

    return env;
}

void emulateKeyPress(MockEnvironment &env, char c) {
    env.mock_key_0.onCall([c]() { return KeyEvent{Key::Text, c}; });
}

struct TestSuitVim : public unittest::StaticTestSuit {
    TestSuitVim() {
        this->testSuitName = "Vim";
        unittest::testResult = &this->testResult;

        for (auto &test : loadVimTests()) {
            newTest(test.name) = [name = test.name, tests = test.tests]() {
                if (name.find("TODO") != std::string::npos) {
                    std::cout << "Test or tested code is not implemented yet"
                              << std::endl;
                    return;
                }
                for (auto &test : tests) {

                    if (test.empty()) {
                        continue;
                    }

                    Registers registers;

                    auto env = createMockEnvironment();
                    env->mock_registers_0.returnValueRef(registers);

                    auto buffer = std::make_shared<Buffer>(test.front());
                    auto editor = Editor{nullptr, buffer};

                    editor.cursor(test.front().position);

                    env->mock_editor_0.returnValueRef(editor);

                    std::cout << "code:\n";

                    auto result = std::string{};

                    for (auto &part : test) {
                        std::cout << "raw: \n'''" << part.input << "'''\n";
                        std::cout << "part:\n'''" << part << "'''\n";
                        std::cout << "command: " << part.command << "\n";
                        std::cout << "mode: " << part.mode << "\n";
                        std::cout << "pos: " << part.position.x() << ", "
                                  << part.position.y() << "\n";

                        if (!result.empty()) {
                            std::cout << "resulting text from previous:\n'''";
                            std::cout << result << "'''" << std::endl;
                            std::cout << "resulting pos: " << editor.cursor()
                                      << std::endl;

                            EXPECT_EQ(part, result);
                            EXPECT_EQ(part.position, editor.cursor());
                        }

                        auto keys = part.command;
                        editor.cursor(part.position);

                        while (!keys.empty()) {
                            emulateKeyPress(*env, keys.front());
                            editor.keyPress(env);
                            keys.erase(0, 1);
                        }

                        result = content({buffer->begin(), buffer->end()});
                        std::cout << "\n";
                    }
                    std::cout.flush();
                }
            };
        }
    }

} TestSuitVim__instance;
