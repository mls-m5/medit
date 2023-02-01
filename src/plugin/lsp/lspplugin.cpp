#include "lspplugin.h"
#include "core/context.h"
#include "core/coreenvironment.h"
#include "core/ijobqueue.h"
#include "core/plugins.h"
#include "files/extensions.h"
#include "lsp/clientnotifications.h"
#include "lsp/lspclient.h"
#include "lsp/requests.h"
#include "lsp/servernotifications.h"
#include "registerdefaultplugins.h"
#include "script/ienvironment.h"
#include "script/iscope.h"
#include "script/scope.h"
#include "syntax/ipalette.h"
#include "views/editor.h"
#include <iostream>

// TODO: Tidy up this file

namespace {

bool shouldProcessFileWithClang(std::filesystem::path path) {
    return isCpp(path) || isCSource(path);
}

std::string createURI(std::filesystem::path path) {
    if (path.empty()) {
        path = "/tmp/tmp.txt";
    }
    return "file://" + std::filesystem::absolute(path).string();
}

std::filesystem::path uriToPath(lsp::URI uri) {
    if (uri.rfind("file://", 0) != 0) {
        return "invalid uri";
    }

    return uri.substr(7);
}

Position toMeditPosition(lsp::Position pos) {
    return Position(pos.character - 1, pos.line - 1);
}

CursorRange toMeditRange(Buffer &b, lsp::Range range) {
    return CursorRange{
        b, toMeditPosition(range.start), toMeditPosition(range.end)};
}

void applyFormat(std::shared_ptr<Buffer> buffer,
                 const std::vector<lsp::DocumentSymbol> &symbols) {
    for (auto &symbol : symbols) {
        buffer->format(toMeditRange(*buffer, symbol.location.range),
                       IPalette::error);
    }
}

/// Locate compile_commands.json file that hopefully is generated by the build
/// system
std::filesystem::path locateCompileCommands() {
    for (auto &it : std::filesystem::recursive_directory_iterator{
             std::filesystem::current_path()}) {
        if (it.path().filename() == "compile_commands.json") {
            return it.path().parent_path();
        }
    }

    return {};
}

lsp::Position meditCursorToPosition(Cursor cursor) {
    auto pos = lsp::Position{};
    pos.line = cursor.y();      // + 1;
    pos.character = cursor.x(); // + 1;
    return pos;
}

Position clangPositionToLspPosition(lsp::Position pos) {
    return Position(pos.character, pos.line);
}

} // namespace

using namespace lsp;

LspPlugin::LspPlugin()
    : _core{CoreEnvironment::instance()} {
    auto previousPath = filesystem::current_path();

    //    setStandardHeaders();

    auto args = std::string{"--log=info "};

    auto compileCommandsPath = locateCompileCommands();

    if (!compileCommandsPath.empty()) {
        args += " --compile-commands-dir=\"" +
                locateCompileCommands().string() + "\"";
        //        filesystem::current_path(compileCommandsPath);
    }

    _client = std::make_unique<LspClient>(args);

    //    filesystem::current_path(previousPath);

    CoreEnvironment::instance().subscribeToBufferEvents(
        [this](BufferEvent e) { bufferEvent(e); });

    _client->request(InitializeParams{}, [](const nlohmann::json &j) {
        //        std::cout << "initialization response:\n";
        //        std::cout << std::setw(2) << j << std::endl;
    });

    _client->subscribe(std::function{
        [/*env = event.env*/](const PublishDiagnosticsParams &params) {
            auto bufferDiagnostics = std::vector<Diagnostics::Diagnostic>{};

            for (auto &item : params.diagnostics) {
                bufferDiagnostics.push_back(Diagnostics::Diagnostic{
                    .source =
                        "clangd", // item.source, could not get this to work
                    .message = item.message,
                    .range = {toMeditPosition(item.range.start),
                              toMeditPosition(item.range.end)},
                });
            }

            CoreEnvironment::instance().publishDiagnostics(
                uriToPath(params.uri),
                "clangd",
                //                params.diagnostics.front().source,
                std::move(bufferDiagnostics));
        }});
}

LspPlugin::~LspPlugin() = default;

void LspPlugin::bufferEvent(BufferEvent &event) {
    if (event.type == BufferEvent::Open) {
        if (!shouldProcessFileWithClang(event.buffer->path())) {
            return;
        }

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

        {
            auto params = DocumentSymbolParams{};
            params.textDocument.uri = createURI(event.buffer->path());

            _client->request(
                params,
                [this, buffer = event.buffer](const nlohmann::json &symbols) {
                    std::cout << symbols << std::endl;
                    //                                 const
                    //                                 DocumentSymbolParams::ReturnT
                    //                                 &symbols) {
                    //                    if (symbols.empty()) {
                    //                        return;
                    //                    }

                    //                    _core.context().guiQueue().addTask(
                    //                        [buffer, symbols] {
                    //                        applyFormat(buffer,
                    //                        symbols); });
                });
        }
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

    if (!shouldProcessFileWithClang(buffer.path())) {
        return;
    }

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

    if (!shouldProcessFileWithClang(scope->editor().buffer().path())) {
        callback({});
        return;
    }

    auto params = CompletionParams{};
    auto &editor = scope->editor();
    params.textDocument.uri = createURI(editor.path());
    auto cursor = editor.cursor();
    params.position.line = cursor.y(); // + 1;
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
            callback(std::move(ret));
        });
}

bool LspComplete::shouldComplete(std::shared_ptr<IScope> env) {
    return shouldProcessFileWithClang(env->editor().path());
}

bool LspHighlight::shouldEnable(filesystem::path path) {
    return shouldProcessFileWithClang(path);
}

void LspHighlight::highlight(std::shared_ptr<IScope> scope) {
    LspPlugin::instance().updateBuffer(scope->editor().buffer());
}

void LspHighlight::update(const IPalette &palette) {}

bool LspNavigation::gotoSymbol(std::shared_ptr<IScope> env) {
    if (!shouldProcessFileWithClang(env->editor().path())) {
        return false;
    }

    auto params = TypeDefinitionParams{};
    params.textDocument.uri = createURI(env->editor().buffer().path());
    params.position = meditCursorToPosition(env->editor().cursor());
    LspPlugin::instance().client().request(
        params, [env](const std::vector<Location> &locations) {
            if (locations.empty()) {
                return;
            }

            env->env().context().guiQueue().addTask([env, locations] {
                auto localEnvironment = std::make_shared<Scope>(env);
                localEnvironment->set(
                    "path", uriToPath(locations.front().uri).string());

                auto pos =
                    clangPositionToLspPosition(locations.front().range.start);

                localEnvironment->set("x", std::to_string(pos.x()));
                localEnvironment->set("y", std::to_string(pos.y()));
                localEnvironment->run(Command{"editor.open"});
            });
        });
    return true;
}