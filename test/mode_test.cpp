#include "keys/bufferkeymap.h"
#include "mls-unit-test/unittest.h"
#include "mock/script/mockenvironment.h"
#include "mock/views/mockwindow.h"
#include "modes/mode.h"
#include "text/buffer.h"
#include "views/editor.h"
#include <memory>

std::shared_ptr<IMode> createTestMode(bool enableDefaultAction = false) {
    auto map = KeyMap{{
        {{Key::Left}, {"test"}},
    }};

    if (enableDefaultAction) {
        map.defaultAction({"hello"});
    }

    auto bufferMap = BufferKeyMap{BufferKeyMap::MapType{
        {{"dw"}, {"delete_word"}},
    }};

    return std::make_shared<Mode>("testmode",
                                  std::move(map),
                                  CursorStyle::Block,
                                  nullptr,
                                  std::move(bufferMap));
}

TEST_SUIT_BEGIN

TEST_CASE("create mode") {
    auto mode = createTestMode();
}

TEST_CASE("match keypress") {
    auto mode = createTestMode();
    auto env = std::make_shared<MockEnvironment>();
    Editor editor{nullptr, std::make_shared<Buffer>()};

    env->mock_editor_0.returnValueRef(editor);
    //        scope->mock_run_1.returnValue(true);

    //    scope->mock_run_1.expectNum(0);

    // Missmatch
    //    scope->environment.mock_key_0.returnValue({Key::F1});
    ASSERT_EQ(false, mode->keyPress(env));

    //    scope->mock_run_1.expectNum(1);

    // Match
    //    scope->environment.mock_key_0.returnValue({Key::Left});
    ASSERT_EQ(true, mode->keyPress(env));
}

TEST_CASE("default action") {
    auto mode = createTestMode(true);
    auto scope = std::make_shared<MockScope>();
    Editor editor{nullptr, std::make_shared<Buffer>()};

    scope->mock_run_1.returnValue(true);
    scope->mock_run_1.expectNum(1);

    // Default actions
    scope->environment.mock_key_0.returnValue({Key::F1});
    ASSERT_EQ(true, mode->keyPress(scope));
}

TEST_CASE("buffered keypress") {
    auto mode = createTestMode();
    auto scope = std::make_shared<MockEnvironment>();
    Editor editor{nullptr, std::make_shared<Buffer>()};

    scope->mock_editor_0.returnValueRef(editor);
    //    scope->mock_run_1.onCall([](auto &&) { return true; });
    scope->mock_run_1.returnValue(true);

    scope->mock_run_1.expectNum(0);

    scope->environment.mock_key_0.returnValue(KeyEvent{Key::Text, 'w'});
    ASSERT_EQ(false, mode->keyPress(scope));

    scope->mock_run_1.expectNum(0);

    scope->environment.mock_key_0.returnValue(KeyEvent{Key::Text, 'd'});
    ASSERT_EQ(true, mode->keyPress(scope));

    scope->mock_run_1.expectNum(1);

    // Second keypress
    scope->environment.mock_key_0.returnValue(KeyEvent{Key::Text, 'w'});
    ASSERT_EQ(true, mode->keyPress(scope));

    // Expect buffer to be cleared
    scope->mock_run_1.expectNum(0);
    scope->environment.mock_key_0.returnValue(KeyEvent{Key::Text, 'w'});
    ASSERT_EQ(false, mode->keyPress(scope));
}

TEST_SUIT_END
