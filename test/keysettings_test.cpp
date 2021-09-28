
#include "keys/keysettings.h"
#include "mls-unit-test/unittest.h"

TEST_SUIT_BEGIN

TEST_CASE("instantiate") {
    KeySettings settings;
}

TEST_CASE("insert key") {
    KeySettings settings;

    auto event = KeyEvent{Key::Escape};

    settings.setBinding("yo", event);

    ASSERT_EQ(settings.action(event), "yo");
}

TEST_SUIT_END
