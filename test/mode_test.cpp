#include "keys/bufferkeymap.h"
#include "mls-unit-test/unittest.h"
#include "modes/mode.h"
#include "script/rootenvironment.h"

std::unique_ptr<IMode> createTestMode() {
    auto map = KeyMap{{
        {{Key::Left}, {"test"}},
    }};
    return std::make_unique<Mode>("testmode", KeyMap{});
}

RootEnvironment createEnvironment() {
    return RootEnvironment();
}

TEST_SUIT_BEGIN

TEST_CASE("create mode") {
    auto mode = createTestMode();
}

TEST_CASE("match keypress") {
    auto mode = createTestMode();
    auto env = createEnvironment();

    env.key({Key::F1});
    ASSERT_EQ(false, mode->keyPress(env, false));

    env.key({Key::Left});
    ASSERT_EQ(false, mode->keyPress(env, false));
}

TEST_SUIT_END
