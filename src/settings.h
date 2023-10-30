#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

enum class UiStyle {
    Terminal,
    Matgui,
    Remote,
    FifoClient,
    FifoServer,
    TcpClient,
    TcpServer,
};

struct Settings {
    std::filesystem::path file;
    UiStyle style = UiStyle::Matgui;
    int port = 4304;
    std::string address = "localhost";

    static constexpr auto helpStr = R"_(
medit text editor

usage:

medit -[flags] [file]

flags:

--gui                 force start with graphic interface
--cli                 start in terminal
--help -h             print this string
--connect [fifo|tcp]  connect to server selected protocol
--host [fifo|tcp]     start server on selected protocol
--remote              test serialize screen
--port -p <port>      set port number for tcp connection
--addr <address>      set the ip address to remote server

    )_";

    Settings(int argc, char **argv) {
        if (argc < 1) {
            return;
        }
        auto args = std::vector<std::string>{argv + 1, argv + argc};

        for (size_t i = 0; i < args.size(); ++i) {
            auto arg = args.at(i);

            if (arg == "--gui") {
                style = UiStyle::Matgui;
            }
            else if (arg == "--cli") {
                style = UiStyle::Terminal;
            }
            else if (arg == "--remote") {
                style = UiStyle::Remote;
            }
            else if (arg == "--connect") {
                auto type = args.at(++i);
                if (type == "fifo") {
                    style = UiStyle::FifoClient;
                }
                else if (type == "tcp") {
                    style = UiStyle::TcpClient;
                }
                else {
                    std::cerr << "invalid connection type (use fifo or tcp) "
                              << type << "\n";
                    std::exit(1);
                }
            }
            else if (arg == "--host") {
                auto type = args.at(++i);
                if (type == "fifo") {
                    style = UiStyle::FifoServer;
                }
                else if (type == "tcp") {
                    style = UiStyle::TcpServer;
                }
                else {
                    std::cerr << "invalid connection type (use fifo or tcp) "
                              << type << "\n";
                    std::exit(1);
                }
            }
            else if (arg == "--addr") {
                address = args.at(++i);
            }
            else if (arg == "--port" || arg == "-p") {
                port = std::stoi(args.at(++i));
            }
            else if (arg == "--help" || arg == "-h") {
                std::cout << helpStr << std::endl;
                std::exit(0);
            }
            else {
                if (arg.rfind("-") != 0) {
                    file = args.at(i);
                }
            }
        }
    }
};
