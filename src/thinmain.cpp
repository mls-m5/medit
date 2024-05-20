
#include "thinmain.h"
#include "files/config.h"
#include "remote/fifoconnection.h"
#include "remote/tcpconnection.h"
#include "screen/deserializescreen.h"
#include "screen/guiscreen.h"

int thinMain(const Settings &settings) {
    auto screen = std::shared_ptr{createGuiScreen()};
    auto dscreen = std::make_shared<DeserializeScreen>(screen);

    auto connection = [&settings]() -> std::shared_ptr<IConnection> {
        if (settings.style == UiStyle::FifoClient) {
            return std::make_shared<FifoConnection>(fifoClientInPath(),
                                                    fifoClientOutPath());
        }
        else if (settings.style == UiStyle::TcpClient) {
            return TcpConnection::connect(settings.address, settings.port);
        }

        throw std::runtime_error{"unsupported connection type"};
    }();

    connect(*connection, *dscreen);
    connection->waitForClose();

    return 0;
}
