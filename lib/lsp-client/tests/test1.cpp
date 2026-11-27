#include "lsp/lspclient.h"
#include "lsp/notifications.h"
#include "lsp/requests.h"
#include <iostream>
#include <sstream>

using namespace lsp;

std::filesystem::path testSrc =
    std::filesystem::absolute("tests/testsrc/test1.cpp");

using namespace std::literals;

template <typename T = nlohmann::json>
void justPrint(const T &json) {
    std::cout << std::setw(4) << nlohmann::json{json} << std::endl;
}

void getSymbolKinds(LspClient &client) {
    auto content =
        (std::ostringstream{} << std::ifstream{testSrc}.rdbuf()).str();

    auto params = DocumentSymbolParams{
        .textDocument = {.uri = "file://" + testSrc.string()}};

    client.request(params, justPrint<std::vector<DocumentSymbol>>);
}

void openDocument(LspClient &client) {
    auto content =
        (std::ostringstream{} << std::ifstream{testSrc}.rdbuf()).str();

    auto params = DidOpenTextDocumentParams{
        .textDocument =
            TextDocumentItem{
                .uri = "file://" + testSrc.string(),
                .languageId = "cpp",
                .version = 1,
                .text = content,
            },
    };

    client.notify(params);

    getSymbolKinds(client);
}

void completeTest(LspClient &client) {
    auto params = CompletionParams{};
    params.textDocument.uri = "file://" + testSrc.string();
    params.position.line = 3;
    client.request(params, justPrint<CompletionList>);
}

void test1(LspClient &client) {
    client.request(InitializeParams{}, [](const nlohmann::json &j) {
        std::cout << "initialization response:\n";
        std::cout << std::setw(2) << j << std::endl;
    });

    // Must wait for server to respond
    // If debugging this might need to be increased to prevent the pipes
    // from being closed prematurely
    std::this_thread::sleep_for(100ms);

    openDocument(client);

    completeTest(client);

    std::this_thread::sleep_for(1s);
}

int main(int argc, char *argv[]) {

    std::string argstr = "clangd ";
    {
        // Use this to pass for example "--compile-commands-dir="
        auto args = std::vector<std::string>{argv + 1, argv + argc};
        for (auto &arg : args) {
            argstr += " " + arg;
        }
    }

    auto client = LspClient{argstr};

    client.subscribe(std::function{[](const PublishDiagnosticsParams &params) {
        std::cout << "hello notification!\n";
        std::cout << "registered notification" << std::endl;
        std::cout << std::setw(2);
        std::cout << nlohmann::json{params} << std::endl;

        for (auto &d : params.diagnostics) {
            std::cout << d.range.start.line << ":" << d.message << std::endl;
        }
    }});

    client.callback([](auto &&j) {
        std::cout << "unregistered notification:\n";
        std::cout << std::setw(2) << j << std::endl;
    });

    test1(client);

    client.shutdown().get();
    client.exit();

    return 0;
}
