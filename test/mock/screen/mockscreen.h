#pragma once

#include "mls-unit-test/mock.h"
#include "screen/iscreen.h"

class MockScreen : public IScreen {
public:
    MOCK_METHOD3(void,
                 draw,
                 (size_t x, size_t y, const FString &str),
                 override);
    MOCK_METHOD0(void, refresh, (), override);
    MOCK_METHOD0(void, clear, (), override);
    MOCK_METHOD2(void, cursor, (size_t x, size_t y), override);
    MOCK_METHOD0(size_t, x, (), const override);
    MOCK_METHOD0(size_t, y, (), const override);
    MOCK_METHOD0(size_t, width, (), const override);
    MOCK_METHOD0(size_t, height, (), const override);
    MOCK_METHOD1(void, title, (std::string title), override);
    MOCK_METHOD1(void, palette, (const Palette &), override);
    MOCK_METHOD3(size_t,
                 addStyle,
                 (const Color &foreground,
                  const Color &background,
                  size_t index),
                 override);
    MOCK_METHOD1(void, cursorStyle, (CursorStyle), override);

    MOCK_METHOD1(void, subscribe, (CallbackT f), override);
    MOCK_METHOD0(void, unsubscribe, (), override);

    MOCK_METHOD0(std::string, clipboardData, (), override);
    MOCK_METHOD1(void, clipboardData, (std::string), override);
};
