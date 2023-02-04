#include "mls-unit-test/unittest.h"
#include "mock/screen/mockscreen.h"
#include "screen/deserializescreen.h"
#include "screen/serializescreen.h"

using namespace std::literals;

struct Fixture {
    std::shared_ptr<MockScreen> output = std::make_shared<MockScreen>();
    std::shared_ptr<DeserializeScreen> ds =
        std::make_shared<DeserializeScreen>(output);
    SerializeScreen ss{ds};
};

TEST_SUIT_BEGIN

TEST_CASE("clear()") {
    auto fixture = Fixture{};

    fixture.output->mock_clear_0.expectNum(1);
    fixture.ss.clear();
}

TEST_CASE("refresh") {
    auto fixture = Fixture{};

    fixture.output->mock_refresh_0.expectNum(1);
    fixture.ss.refresh();
}

TEST_SUIT_END
