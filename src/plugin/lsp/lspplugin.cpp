#include "lspplugin.h"
#include "core/context.h"
#include "core/coreenvironment.h"
#include "core/ijobqueue.h"
#include "core/plugins.h"
#include "files/project.h"
#include "lsp/clientnotifications.h"
#include "lsp/lspclient.h"
#include "lsp/requests.h"
#include "lsp/servernotifications.h"
#include "lspconfiguration.h"
#include "script/ienvironment.h"
#include "script/standardcommands.h"
#include "syntax/basichighligting.h"
#include "syntax/palette.h"
#include "text/changes.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include <filesystem>
#include <future>
#include <iomanip>
#include <iostream>
#include <stdexcept>

// TODO: Tidy up this file

namespace {

std::string pathToUri(std::filesystem::path path) {
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

/// @param offest   of the input is offsetted by 1 character and need sto be
/// adjusted for
Position toMeditPosition(lsp::Position pos, bool offset = false) {
    return Position(pos.character - 1 * offset, pos.line - 1 * offset);
}

CursorRange toMeditRange(Buffer &b, lsp::Range range, bool offset = false) {
    return CursorRange{b,
                       toMeditPosition(range.start, offset),
                       toMeditPosition(range.end, offset)};
}

lsp::Position meditCursorToPosition(Cursor cursor) {
    auto pos = lsp::Position{};
    pos.line = cursor.y();      // + 1;
    pos.character = cursor.x(); // + 1;
    return pos;
}

Position clangPositionToMeditPosition(lsp::Position pos) {
    return Position(pos.character, pos.line);
}

} // namespace

using namespace lsp;

void LspPlugin::init() {
    _config = LspConfiguration{_core->project().projectExtension()};

    _client = std::make_unique<LspClient>(_config.command);

    _core->files().subscribeToBufferEvents(
        [this](BufferEvent e) { bufferEvent(e); }, nullptr, this);

    auto initializedPromise = std::promise<void>{};
    auto initializedFuture = initializedPromise.get_future();

    _client->request(
        InitializeParams{},
        [&initializedPromise](const nlohmann::json &j) {
            //            std::cout << "initialization response:\n";
            //            std::cout << std::setw(2) << j << std::endl;
            initializedPromise.set_value();
        },
        [&initializedPromise](auto &&j) {
            std::cerr << "error\n";
            std::cerr << j << "\n";
            initializedPromise.set_value();
        });

    _client->subscribe(
        std::function{[this](const PublishDiagnosticsParams &params) {
            auto bufferDiagnostics = std::vector<Diagnostics::Diagnostic>{};

            for (auto &item : params.diagnostics) {
                bufferDiagnostics.push_back(Diagnostics::Diagnostic{
                    .source =
                        "clangd", // item.source, could not get this to work
                    .message = item.message,
                    .range = {toMeditPosition(item.range.start, true),
                              toMeditPosition(item.range.end, true)},
                });
            }

            _core->context().guiQueue().addTask(
                [this, path = uriToPath(params.uri), bufferDiagnostics] {
                    _core->files().publishDiagnostics(
                        path,
                        "clangd",
                        //                params.diagnostics.front().source,
                        std::move(bufferDiagnostics));
                });
        }});

    _client->callback([](auto j) {
        std::cerr << "Unhandled callback\n";
        std::cerr << std::setw(4) << j << "\n";
    });

    initializedFuture.get();
}

LspPlugin::LspPlugin() = default;

LspPlugin::~LspPlugin() {
    _client->shutdown().get();
}

void LspPlugin::bufferEvent(BufferEvent &event) {
    if (event.type == BufferEvent::Open) {
        if (!_config.isFileSupported(event.buffer->path())) {
            return;
        }

        auto content = event.buffer->text();

        auto params = DidOpenTextDocumentParams{
            .textDocument =
                TextDocumentItem{
                    .uri = pathToUri(event.path),
                    .languageId = "cpp",
                    .version = event.buffer->history().revision(),
                    .text = content,
                },
        };

        _client->notify(params);
        requestSemanticsToken(event.buffer);
    }
    if (event.type == BufferEvent::Close) {
        if (!_config.isFileSupported(event.path)) {
            return;
        }

        auto params = DidCloseTextDocumentParams{
            .textDocument =
                {
                    .uri = pathToUri(event.path),
                },
        };

        _client->notify(params);
        //        requestSemanticsToken(event.buffer);
    }
}

void LspPlugin::registerPlugin(CoreEnvironment &core, Plugins &plugins) {
    // TODO: Fix this someday, its ugly
    auto lsp = std::make_shared<LspPlugin>();
    lsp->_core = &core; // Replace for with constructor
    lsp->init();
    //    LspPlugin::instance()._core = &core;
    //    LspPlugin::instance().init();
    plugins.loadPlugin<LspNavigation>(lsp);
    plugins.loadPlugin<LspHighlight>(lsp);
    plugins.loadPlugin<LspComplete>(lsp);
    plugins.loadPlugin<LspRename>(lsp);
}

void LspPlugin::handleSemanticsTokens(std::shared_ptr<Buffer> buffer,
                                      std::vector<long> data) {

    struct Item {
        long *data;

        /// at index 5*i - deltaLine: token line number, relative to the
        /// previous
        ///    token
        long deltaLine = data[0];

        /// at index 5*i+1 - deltaStart: token start character, relative to the
        ///    previous token (relative to 0 or the previous token’s start if
        ///    they are on the same line)
        long deltaStart = data[1];

        /// at index 5*i+2 - length: the length of the token.
        long length = data[2];

        /// at index 5*i+3 - tokenType: will be looked up in
        ///    SemanticTokensLegend.tokenTypes. We currently ask that tokenType
        ///    < 65536.
        long tokenType = data[3];

        /// at index 5*i+4 - tokenModifiers: each set bit will be looked up in
        ///    SemanticTokensLegend.tokenModifiers
        long tokenModifiers = data[4];
    };

    //    format(all(*buffer), IPalette::standard);

    BasicHighlighting::highlightStatic(*buffer);

    Cursor cur = buffer->begin();

    static std::unordered_map<lsp::SemanticTokenTypes, Palette::BasicPalette>
        translation = {
            {SemanticTokenTypes::Type, Palette::type},
            {SemanticTokenTypes::Class, Palette::type},
            {SemanticTokenTypes::Enum, Palette::type},
            {SemanticTokenTypes::Interface, Palette::type},
            {SemanticTokenTypes::Struct, Palette::type},
            {SemanticTokenTypes::TypeParameter, Palette::type},
            {SemanticTokenTypes::Parameter, Palette::identifier},
            {SemanticTokenTypes::Variable, Palette::identifier},
            {SemanticTokenTypes::Property, Palette::identifier},
            {SemanticTokenTypes::EnumMember, Palette::identifier},
            {SemanticTokenTypes::Event, Palette::identifier},
            {SemanticTokenTypes::Function, Palette::identifier},
            {SemanticTokenTypes::Method, Palette::identifier},
            {SemanticTokenTypes::Macro, Palette::comment},
            {SemanticTokenTypes::Keyword, Palette::identifier},
            {SemanticTokenTypes::Modifier, Palette::statement},
            {SemanticTokenTypes::Comment, Palette::standard},
            {SemanticTokenTypes::String, Palette::string},
            {SemanticTokenTypes::Number, Palette::constant},
            {SemanticTokenTypes::Regexp, Palette::standard},
            {SemanticTokenTypes::Operator, Palette::standard},
        };

    auto translate = [](SemanticTokenTypes t) {
        if (auto f = translation.find(t); f != translation.end()) {
            return f->second;
        }
        return Palette::standard;
    };

    for (size_t i = 0; i < data.size(); i += 5) {
        auto item = Item{data.data() + i};

        cur.y(cur.y() + item.deltaLine);
        if (item.deltaLine) {
            cur.x(item.deltaStart);
        }
        else {
            cur.x(item.deltaStart + cur.x());
        }

        Cursor end = cur;
        end.x(cur.x() + item.length);

        format({cur, end},
               translate(static_cast<SemanticTokenTypes>(item.tokenType)));
    }

    //    buffer->emitChangeSignal();
}

void LspPlugin::requestSemanticsToken(std::shared_ptr<Buffer> buffer) {
    auto params = SemanticTokensParams{};
    params.textDocument.uri = pathToUri(buffer->path());

    _client->request(params, [this, buffer](SemanticTokens data) {
        _core->context().guiQueue().addTask([buffer, data, this] {
            handleSemanticsTokens(buffer, std::move(data.data));
        });
    });
}

void LspPlugin::updateBuffer(Buffer &buffer) {
    // TODO: Only update buffers with changed revision

    if (!_config.isFileSupported(buffer.path())) {
        return;
    }

    auto &oldVersion = _bufferVersions[buffer.path().string()];

    if (oldVersion == buffer.history().revision()) {
        return;
    }

    oldVersion = buffer.history().revision();

    auto params = DidChangeTextDocumentParams{};
    params.textDocument.uri = pathToUri(buffer.path());
    params.textDocument.version = buffer.history().revision();
    params.contentChanges.push_back(TextDocumentContentChangeEvent{
        .text = buffer.text(), // TODO: Only do partial updates
    });
    _client->notify(params);

    requestSemanticsToken(buffer.shared_from_this());
}

void LspComplete::list(std::shared_ptr<IEnvironment> scope,
                       CompleteCallbackT callback) {

    if (!_lsp->config().isFileSupported(scope->editor().buffer().path())) {
        callback({});
        return;
    }

    auto params = CompletionParams{};
    auto &editor = scope->editor();
    params.textDocument.uri = pathToUri(editor.path());
    auto cursor = editor.cursor();
    params.position.line = cursor.y(); // + 1;
    params.position.character = cursor.x() + 1;

    _lsp->updateBuffer(scope->editor().buffer());

    _lsp->client().request(params, [callback](lsp::CompletionList result) {
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

bool LspComplete::shouldComplete(std::shared_ptr<IEnvironment> env) {
    return _lsp->config().isFileSupported(env->editor().path());
}

bool LspHighlight::shouldEnable(std::filesystem::path path) {
    return _lsp->config().isFileSupported(path);
}

void LspHighlight::highlight(Buffer &buffer) {
    _lsp->updateBuffer(buffer);
}

bool LspNavigation::gotoSymbol(std::shared_ptr<IEnvironment> env) {
    if (!_lsp->config().isFileSupported(env->editor().path())) {
        return false;
    }

    auto params = TypeDefinitionParams{};
    params.textDocument.uri = pathToUri(env->editor().buffer().path());
    params.position = meditCursorToPosition(env->editor().cursor());
    _lsp->client().request(
        params, [env](const std::vector<Location> &locations) {
            if (locations.empty()) {
                return;
            }

            env->context().guiQueue().addTask([env, locations] {
                auto pos =
                    clangPositionToMeditPosition(locations.front().range.start);

                env->standardCommands().open(
                    env->shared_from_this(),
                    uriToPath(locations.front().uri).string(),
                    pos.x(),
                    pos.y());
            });
        });
    return true;
}

bool LspRename::shouldEnable(std::filesystem::path path) const {
    return (_lsp->config().isFileSupported(path));
}

bool LspRename::doesSupportPrepapre() {
    // TODO: Depend on servercapabilities in the future
    return true;
}

bool LspRename::prepare(std::shared_ptr<IEnvironment> env,
                        std::function<void(PrepareCallbackArgs)> callback) {
    if (!_lsp->config().isFileSupported(env->editor().path())) {
        return false;
    }

    auto params = PrepareRenameParams{};
    params.textDocument.uri = pathToUri(env->editor().buffer().path());
    params.position = meditCursorToPosition(env->editor().cursor());

    _lsp->client().request(
        params,
        [env, callback](const Range &range) {
            env->context().guiQueue().addTask([env, range, callback] {
                auto args = IRename::PrepareCallbackArgs{
                    toMeditPosition(range.start), toMeditPosition(range.end)};
                callback(args);
            });
        },
        [env, callback](const auto &j) {
            env->context().guiQueue().addTask([env, callback] {
                //            std::cout << "error: " << j << std::endl; //
                auto args = IRename::PrepareCallbackArgs{};
                callback(args);
            });
        });

    return true;
}

bool LspRename::rename(std::shared_ptr<IEnvironment> env,
                       RenameArgs args,
                       std::function<void(const Changes &)> callback) {
    if (!_lsp->config().isFileSupported(env->editor().path())) {
        return false;
    }

    auto params = RenameParams{};
    params.textDocument.uri = pathToUri(env->editor().buffer().path());
    params.position = meditCursorToPosition(env->editor().cursor());
    params.newName = args.newName;

    _lsp->client().request(params, [env, callback](const WorkspaceEdit &edits) {
        if (edits.changes.empty()) {
            return;
        }

        auto changes = Changes{};

        for (auto &file : edits.changes) {
            auto fileChanges = decltype(Changes::FileChanges::changes){};

            for (auto &lspChange : file.second) {
                auto change = Changes::Change{};
                change.begin = toMeditPosition(lspChange.range.start);
                change.end = toMeditPosition(lspChange.range.end);
                change.newText = lspChange.newText;

                fileChanges.push_back(change);
            }
            changes.changes.emplace_back(Changes::FileChanges{
                .file = std::filesystem::relative(
                    uriToPath(file.first), env->project().settings().root),
                .changes = std::move(fileChanges),
            });
        }

        env->context().guiQueue().addTask(
            [env, changes = std::move(changes), callback] {
                callback(std::move(changes));
            });
    });
    return true;
}
