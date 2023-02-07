
#include "thinmain.h"
#include "remote/fifoconnection.h"
#include "remote/fifofile.h"
#include "remote/tcpconnection.h"
#include "screen/deserializescreen.h"
#include "screen/guiscreen.h"

int thinMain(const Settings &settings) {
    auto screen = std::make_shared<GuiScreen>();
    auto dscreen = std::make_shared<DeserializeScreen>(screen);

    if (settings.style == UiStyle::FifoClient) {
        auto receiver =
            FifoConnection{FifoFile::clientInPath, FifoFile::clientOutPath};
        // Connect in reverse order
        dscreen->subscribe([&receiver](auto data) { receiver.write(data); });
        receiver.subscribe([&dscreen](auto data) { dscreen->write(data); });
        receiver.waitForClose();
    }
    else if (settings.style == UiStyle::TcpClient) {
        auto client = TcpConnection::connect(settings.address, settings.port);

        dscreen->subscribe([&client](auto data) { client->write(data); });
        client->subscribe([&dscreen](auto data) { dscreen->write(data); });
        client->waitForClose();
    }

    return 0;
}
