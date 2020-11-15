#include "keys/bufferkeymap.h"
#include "mls-unit-test/unittest.h"
#include "mock/script/mockenvironment.h"
#include "modes/mode.h"
#include "views/editor.h"

std::unique_ptr<IMode> createTestMode(bool enableDefaultAction = false) {
    auto map = KeyMap{{
        {{Key::Left}, {"test"}},
    }};

    if (enableDefaultAction) {
        map.defaultAction({"hello"});
    }

    auto bufferMap = BufferKeyMap{BufferKeyMap::MapType{
        {{"dw"}, {"delete_word"}},
    }};

    return std::make_unique<Mode>(
        "testmode", std::move(map), nullptr, std::move(bufferMap));
}

TEST_SUIT_BEGIN

TEST_CASE("create mode") {
    auto mode = createTestMode();
}

TEST_CASE("match keypress") {
    auto mode = createTestMode();
    auto env = MockEnvironment();
    auto editor = Editor{};

    env.mock_editor_0.returnValueRef(editor);
    //    env.mock_run_1.onCall([](auto &&) { return true; });
    env.mock_run_1.returnValue(true);

    env.mock_run_1.expectNum(0);

    // Missmatch
    env.mock_key_0.returnValue({Key::F1});
    ASSERT_EQ(false, mode->keyPress(env));

    env.mock_run_1.expectNum(1);

    // Match
    env.mock_key_0.returnValue({Key::Left});
    ASSERT_EQ(true, mode->keyPress(env));
}

TEST_CASE("default action") {
    auto mode = createTestMode(true);
    auto env = MockEnvironment();
    auto editor = Editor{};

    env.mock_run_1.returnValue(true);
    env.mock_run_1.expectNum(1);

    // Default actions
    env.mock_key_0.returnValue({Key::F1});
    ASSERT_EQ(true, mode->keyPress(env));
}

TEST_CASE("buffered keypress") {
    auto mode = createTestMode();
    auto env = MockEnvironment{};
    auto editor = Editor{};

    env.mock_editor_0.returnValueRef(editor);
    //    env.mock_run_1.onCall([](auto &&) { return true; });
    env.mock_run_1.returnValue(true);

    env.mock_run_1.expectNum(0);

    env.mock_key_0.returnValue(KeyEvent{Key::Text, 'w'});
    ASSERT_EQ(false, mode->keyPress(env));

    env.mock_run_1.expectNum(0);

    env.mock_key_0.returnValue(KeyEvent{Key::Text, 'd'});
    ASSERT_EQ(true, mode->keyPress(env));

    env.mock_run_1.expectNum(1);

    // Second keypress
    env.mock_key_0.returnValue(KeyEvent{Key::Text, 'w'});
    ASSERT_EQ(true, mode->keyPress(env));

    // Expect buffer to be cleared
    env.mock_run_1.expectNum(0);
    env.mock_key_0.returnValue(KeyEvent{Key::Text, 'w'});
    ASSERT_EQ(false, mode->keyPress(env));
}

TEST_SUIT_END
