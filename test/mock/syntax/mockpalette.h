#pragma once

#include "mls-unit-test/mock.h"
#include "syntax/ipalette.h"

class MockPalette : public IPalette {
public:
    MOCK_CONST_METHOD1(FormatType, getFormat, (std::string), override);
};
