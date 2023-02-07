
#include "thinmain.h"
#include "remote/fifofile.h"
#include "remote/fiforeceiver.h"
#include "screen/deserializescreen.h"
#include "screen/guiscreen.h"

int thinMain(const Settings &settings) {
    auto screen = std::make_shared<GuiScreen>();
    auto dscreen = std::make_shared<DeserializeScreen>(screen);
    auto receiver =
        FifoConnection{FifoFile::clientInPath, FifoFile::clientOutPath};

    // Connect in reverse order
    dscreen->subscribe([&receiver](auto data) { receiver.write(data); });
    receiver.subscribe([&dscreen](auto data) { dscreen->write(data); });

    receiver.waitForClose();

    return 0;
}
