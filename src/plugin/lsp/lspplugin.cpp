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
#include "script/iscope.h"
#include "views/editor.h"

// TODO: Tidy up this file

namespace {

std::string createURI(std::filesystem::path path) {
    if (path.empty()) {
        path = "/tmp/tmp.txt";
    }
    return "file://" + std::filesystem::absolute(path).string();
}

} // namespace

using namespace lsp;

LspPlugin::LspPlugin()
    : _client{std::make_unique<LspClient>("--log=verbose")} {
    CoreEnvironment::instance().subscribeToBufferEvents(
        [this](BufferEvent e) { bufferEvent(e); });

    _client->request(InitializeParams{}, [](const nlohmann::json &j) {
        //        std::cout << "initialization response:\n";
        //        std::cout << std::setw(2) << j << std::endl;
    });

    _client->subscribe(std::function{
        [/*env = event.env*/](const PublishDiagnosticsParams &params) {
            //            auto &console = env->console();

            //            auto &consoleBuffer = console.buffer();

            //            for (auto &d : params.diagnostics) {
            //                env->context().guiQueue().addTask([text =
            //                d.message, env] {
            //                    env->console().buffer().pushBack(text);
            //                    env->console().cursor({0, 100000000});
            //                });
            //            }
        }});
}

LspPlugin::~LspPlugin() = default;

void LspPlugin::bufferEvent(BufferEvent &event) {
    if (event.type == BufferEvent::Open) {
        auto content = event.buffer->text();

        auto params = DidOpenTextDocumentParams{
            .textDocument =
                TextDocumentItem{
                    .uri = createURI(event.buffer->path()),
                    .languageId = "cpp",
                    .version = event.buffer->history().revision(),
                    .text = content,
                },
        };

        _client->notify(params);
    }
}

void LspPlugin::registerPlugin() {
    LspPlugin::instance();
    registerNavigation<LspNavigation>();
    registerHighlighting<LspHighlight>();
    registerCompletion<LspComplete>();
}

void LspPlugin::updateBuffer(Buffer &buffer) {
    // TODO: Only update buffers with changed revision

    auto &oldVersion = _bufferVersions[buffer.path().string()];

    if (oldVersion == buffer.history().revision()) {
        return;
    }

    oldVersion = buffer.history().revision();

    auto params = DidChangeTextDocumentParams{};
    params.textDocument.uri = createURI(buffer.path());
    params.textDocument.version = buffer.history().revision();
    params.contentChanges.push_back(TextDocumentContentChangeEvent{
        .text = buffer.text(), // TODO: Only do partial updates
    });
    _client->notify(params);
}

void LspComplete::list(std::shared_ptr<IScope> scope,
                       CompleteCallbackT callback) {
    auto params = CompletionParams{};
    auto &editor = scope->editor();
    params.textDocument.uri = createURI(editor.path());
    auto cursor = editor.cursor();
    params.position.line = cursor.y() + 1;
    params.position.character = cursor.x() + 1;

    LspPlugin::instance().updateBuffer(scope->editor().buffer());

    LspPlugin::instance().client().request(
        params, [callback](lsp::CompletionList result) {
            auto ret = CompletionList{};
            for (auto &item : result.items) {

                auto range = TextEdit::Range{
                    .begin = ::Position(item.textEdit.range.start.character - 1,
                                        item.textEdit.range.start.line - 1),
                    .end = ::Position(item.textEdit.range.end.character - 1,
                                      item.textEdit.range.end.line - 1),
                };

                ret.push_back(ICompletionSource::CompletionItem{
                    .name = item.label, // TODO: Redo this class
                    .description = item.detail,
                    .filterText = item.filterText,
                    .edit =
                        {
                            .range = range,
                            .text = item.textEdit.newText,
                        },
                });
            }
            //            callback({{"hello", "there"}, {"you", "!"}});
            callback(std::move(ret));
        });
}

void LspHighlight::highlight(std::shared_ptr<IScope> scope) {
    LspPlugin::instance().updateBuffer(scope->editor().buffer());
}

void LspHighlight::update(const IPalette &palette) {}
