#include "keys/bufferkeymap.h"
#include "mls-unit-test/unittest.h"
#include "mock/script/mockienvironment.h"
#include "modes/mode.h"
#include "views/editor.h"

std::unique_ptr<IMode> createTestMode() {
    auto map = KeyMap{{
        {{Key::Left}, {"test"}},
    }};

    return std::make_unique<Mode>("testmode", KeyMap{});
}

TEST_SUIT_BEGIN

TEST_CASE("create mode") {
    auto mode = createTestMode();
}

TEST_CASE("match keypress") {
    auto mode = createTestMode();
    auto env = MockIEnvironment();
    auto editor = Editor{};

    env.mock_editor_0.returnValueRef(editor);
    env.mock_run_1.onCall([](auto &&) { return true; });

    env.mock_key_0.returnValue({Key::F1});
    ASSERT_EQ(false, mode->keyPress(env, false));

    env.mock_key_0.returnValue({Key::Left});
    ASSERT_EQ(false, mode->keyPress(env, false));
}

TEST_CASE("buffered keypress") {
    // Todo: implement
}

TEST_SUIT_END
