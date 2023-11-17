/*
 * Scripted tests. The tests are loaded from a file
 * */
#include "core/registers.h"
#include "keys/event.h"
#include "mls-unit-test/expect.h"
#include "mls-unit-test/unittest.h"
#include "mock/script/mockenvironment.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "modes/visualmode.h"
#include "script/ienvironment.h"
#include "script/standardcommands.h"
#include "text/buffer.h"
#include "text/cursorrangeops.h"
#include "text/position.h"
#include "views/editor.h"
#include <filesystem>
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

std::string replaceSpecialKeys(std::string str) {
    auto list = std::vector<std::pair<std::string, std::string>>{
        {"<esc>", "\027"},
    };
    for (auto &rep : list) {
        for (size_t f = 0; ((f = str.find(rep.first)) != std::string::npos);
             ++f) {
            str.replace(f, rep.first.size(), rep.second);
        }
    }
    return str;
}

std::string unescapeCommand(std::string raw) {
    auto ret = std::string{};
    bool isEscape = false;
    for (auto c : raw) {
        if (c == '\\') {
            isEscape = true;
            continue;
        }

        if (isEscape) {
            isEscape = false;

            switch (c) {
            case 'b':
                c = '\b';
                break;
            case 'n':
                c = '\n';
                break;
            case 't':
                c = '\t';
                break;
            }
        }

        ret.push_back(c);
    }

    return replaceSpecialKeys(ret);
}

std::string translateVimMode(char c) {
    switch (c) {
    case 'I':
        return "insert";
    case 'V':
        return "visual";
    case 'B':
        return "visual block";
    }
    return "normal";
}

std::shared_ptr<IMode> createMode(char c) {
    switch (c) {
    case 'I':
        return createInsertMode();
    case 'V':
        return createVisualMode(false);
    case 'B':
        return createVisualMode(true);
    }
    return createNormalMode();
}

std::pair<std::string, std::string> diffText(std::string a, std::string b) {
    if (a.empty() || b.empty()) {
        return {};
    }

    while (!(a.empty() || b.empty())) {
        if (a.front() == b.front()) {
            a.erase(0, 1);
            b.erase(0, 1);
            continue;
        }
        break;
    }
    while (!(a.empty() || b.empty())) {
        if (a.back() == b.back()) {
            a.pop_back();
            b.pop_back();
            continue;
        }
        break;
    }

    return {a, b};
}

} // namespace

struct VimCodePart : public std::string {
    std::string input;
    std::string mode;
    std::string command;
    Position position;
    std::optional<Position> anchor;

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

            command = unescapeCommand(match[3]);
            mode = match[1];

            // TODO: Handle newlines
            position = getPosition(match.position());

            erase(match.position(), match[0].str().size());
        }

        if (auto f = find("[VA]"); f != std::string::npos) {
            anchor = getPosition(f);

            if (anchor->y() == position.y()) {
                if (anchor->x() < position.x()) {
                    position.x(position.x() - 4);
                }
            }

            erase(f, 4);
        }
    }

    Position getPosition(size_t index) {
        size_t col = 0;
        size_t line = 0;
        for (size_t i = 0; i < size() && i < index; ++i) {
            auto c = at(i);
            if (c == '\n') {
                ++line;
                col = 0;
            }
            else {
                ++col;
            }
        }

        return {col, line};
    }
};

struct VimSubTest : public std::vector<VimCodePart> {};

struct VimTest {
    std::string name;
    std::vector<VimSubTest> tests;
};

/// Split file up in sub-tests
std::vector<VimTest> loadVimTests() {

    auto ret = std::vector<VimTest>{};

    for (auto &it : std::filesystem::directory_iterator{"test/res"}) {
        if (!it.path().filename().string().starts_with("vim") ||
            it.path().extension() != ".md") {
            continue;
        }
        auto file = std::ifstream{it.path()};
        bool isCode = false;

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

    env->mock_standardCommands_0.returnValueRef(StandardCommands::get());
    env->mock_statusMessage_1.onCall(
        [](auto msg) { std::cout << "status message: " << msg << "\n"; });

    return env;
}

void emulateKeyPress(MockEnvironment &env, char c) {
    auto key = Key::Text;

    switch (c) {
    case '\b':
        key = Key::Backspace;
        c = 0;
        break;
    case '\027':
        key = Key::Escape;
        c = 0;
        break;
    }

    env.mock_key_0.onCall([c, key]() { return KeyEvent{key, c}; });
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

                    auto result = std::string{};
                    auto previous = std::string{};

                    for (auto &part : test) {
                        if (false) {
                            std::cout << "raw: \n'''" << part.input << "'''\n";
                            std::cout << "part:\n'''" << part << "'''\n";
                            std::cout << "command: " << part.command << "\n";
                            std::cout << "mode: " << part.mode << "\n";
                            std::cout << "pos: " << part.position.x() << ", "
                                      << part.position.y() << "\n";
                        }

                        if (!result.empty()) {
                            if (false) {
                                std::cout
                                    << "resulting text from previous:\n'''";
                                std::cout << result << "'''" << std::endl;
                                std::cout
                                    << "resulting pos: " << editor.cursor()
                                    << std::endl;
                            }

                            auto diff = diffText(previous, result);

                            EXPECT_EQ(part, result);
                            EXPECT_EQ(part.position, editor.cursor());
                            if (!part.mode.empty()) {
                                EXPECT_EQ(translateVimMode(part.mode.front()),
                                          editor.mode().name());
                            }
                            if (part.mode != "I") {
                                EXPECT_EQ(
                                    registers.load(standardRegister).value,
                                    diff.first);
                            }

                            if (part.anchor) {
                                EXPECT_TRUE(editor.selectionAnchor());
                                EXPECT_EQ(*editor.selectionAnchor(),
                                          *part.anchor);
                            }
                        }

                        auto keys = part.command;
                        editor.cursor(part.position);

                        if (!part.mode.empty()) {
                            editor.mode(createMode(part.mode.front()));
                        }
                        while (!keys.empty()) {
                            emulateKeyPress(*env, keys.front());
                            editor.keyPress(env);
                            keys.erase(0, 1);
                        }

                        result = content({buffer->begin(), buffer->end()});
                        previous = part;
                        std::cout << "\n";
                    }
                    std::cout.flush();
                }
            };
        }
    }

} TestSuitVim__instance;
