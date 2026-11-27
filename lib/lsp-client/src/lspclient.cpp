#include "lsp/lspclient.h"
#include <sstream>

lsp::LspClient::LspClient(std::string command)
    : _connection{command,
                  [this](auto &in) { inputThread(in); },
                  [this]() {
                      if (_unexpectedExitCallback) {
                          _unexpectedExitCallback();
                      }
                  }} {}

lsp::LspClient::~LspClient() {
    _abort = true;
}

std::future<void> lsp::LspClient::shutdown() {

    auto exitPromise = std::make_shared<std::promise<void>>();

    if (!_connection) {
        exitPromise->set_value(); // Do not wait for anything if the connection
                                  // is closed
        return exitPromise->get_future();
    }

    auto callbackF = [this, exitPromise](nlohmann::json j) {
        exit(); //
        exitPromise->set_value();
    };

    request("shutdown", nlohmann::json{}, callbackF, callbackF);

    return exitPromise->get_future();
}

void lsp::LspClient::exit() {
    _abort = true;
    if (!_connection) {
        return;
    }
    notify("exit", nlohmann::json{});
}

void lsp::LspClient::inputThread(std::istream &in) {
    using namespace std::literals;

    size_t contentLength = 0;

    auto contentLengthStr = "Content-Length: "sv;
    auto contentTypeStr = "Content-Type: "sv;

    for (std::string line; std::getline(in, line);) {
        if (_abort) {
            break;
        }
        if (line.empty()) {
            continue;
        }

        if (line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty() || contentLength) {
            auto json = nlohmann::json{};
            in >> json;

            handle(json);

            contentLength = 0;

            continue;
        }

        if (line.rfind(contentLengthStr, 0) == 0) {
            contentLength = std::stol(line.substr(contentLengthStr.size()));
        }

        if (_abort) {
            break;
        }
    }
}

void lsp::LspClient::send(const nlohmann::json &json) {
    const auto str = [&] {
        auto ss = std::stringstream{};
        ss << std::setw(2) << json;
        return ss.str();
    }();

    if (!_connection) {
        throw std::runtime_error{"clangd is not running"};
    }

    auto ss = std::ostringstream{};

    ss << "Content-Length: " << str.length() + 1 << "\r\n";
    ss << "\r\n";
    ss << str;
    ss.flush();

    if (_connection.send(ss.str())) {
        _handling.failAll();
    }
}
