#include "lspplugin.h"
#include "core/context.h"
#include "core/coreenvironment.h"
#include "core/ijobqueue.h"
#include "core/plugins.h"
#include "lsp/clientnotifications.h"
#include "lsp/lspclient.h"
#include "lsp/requests.h"
#include "lsp/servernotifications.h"
#include "registerdefaultplugins.h"
#include "script/ienvironment.h"
#include "views/editor.h"

// TODO: Tidy up this file

using namespace lsp;

LspPlugin::LspPlugin()
    : _client{std::make_unique<LspClient>("")} {
    CoreEnvironment::instance().subscribeToBufferEvents(
        [this](BufferEvent e) { bufferEvent(e); });

    _client->request(InitializeParams{}, [](const nlohmann::json &j) {
        //        std::cout << "initialization response:\n";
        //        std::cout << std::setw(2) << j << std::endl;
    });
}

LspPlugin::~LspPlugin() = default;

void LspPlugin::bufferEvent(BufferEvent &event) {
    if (event.type == BufferEvent::Open) {
        auto content = event.buffer->text();

        auto params = DidOpenTextDocumentParams{
            .textDocument =
                TextDocumentItem{
                    .uri = "file://" + event.buffer->path().string(),
                    .languageId = "cpp",
                    .version = 1,
                    .text = content,
                },
        };

        _client->notify(params);

        _client->subscribe(std::function{
            [env = event.env](const PublishDiagnosticsParams &params) {
                auto &console = env->console();

                auto &consoleBuffer = console.buffer();

                for (auto &d : params.diagnostics) {
                    env->context().guiQueue().addTask([text = d.message, env] {
                        env->console().buffer().pushBack(text);
                        env->console().cursor({0, 100000000});
                    });
                }
            }});
    }
}

void LspPlugin::registerPlugin() {
    registerNavigation<LspNavigation>();
    registerHighlighting<LspHighlight>();
    registerCompletion<LspComplete>();
}

void LspComplete::list(std::shared_ptr<IScope>, CompleteCallbackT callback) {
    callback({{"hello", "there"}, {"you", "!"}});
}
