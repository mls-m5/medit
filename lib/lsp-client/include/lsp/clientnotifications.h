/// Notifications sent from the client to the server
/// Notifications is messages sent that does not need a response
#pragma once

#include "lsptypes.h"

namespace lsp {

enum TextDocumentSyncKind {
    None = 0,
    Full = 1,
    Incremental = 2,
};

struct DidOpenTextDocumentParams {
    static constexpr std::string_view method = "textDocument/didOpen";
    /**
     * The document that was opened.
     */
    TextDocumentItem textDocument;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DidOpenTextDocumentParams, textDocument)

struct DidCloseTextDocumentParams {
    static constexpr std::string_view method = "textDocument/didClose";
    /**
     * The document that was closed.
     */
    TextDocumentIdentifier textDocument;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DidCloseTextDocumentParams, textDocument)

struct TextDocumentContentChangeEvent {
    std::optional<Range> range;
    std::optional<Integer> rangeLength; // Deprecated from 3.0 dont use
    std::string text;
};

void to_json(nlohmann::json &j, const TextDocumentContentChangeEvent &e) {
    if (e.range) {
        to_json(j["range"], *e.range);
    }
    if (e.rangeLength) {
        j["rangeLength"] = *e.rangeLength;
    }
    j["text"] = e.text;
}

void from_json(const nlohmann::json &j, TextDocumentContentChangeEvent &e) {
    if (auto it = j.find("range"); it != j.end()) {
        e.range = it->get<Range>();
    }
    if (auto it = j.find("rangeLength"); it != j.end()) {
        e.rangeLength = *it;
    }
    j.at("text").get_to(e.text);
}

// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(TextDocumentContentChangeEvent,
//                                                 range,
//                                                 rangeLength,
//                                                 text);

struct DidChangeTextDocumentParams {
    static constexpr std::string_view method = "textDocument/didChange";
    /**
     * The document that did change. The version number points
     * to the version after all provided content changes have
     * been applied.
     */
    VersionedTextDocumentIdentifier textDocument;

    /**
     * The actual content changes. The content changes describe single state
     * changes to the document. So if there are two content changes c1 (at
     * array index 0) and c2 (at array index 1) for a document in state S then
     * c1 moves the document from S to S' and c2 from S' to S''. So c1 is
     * computed on the state S and c2 is computed on the state S'.
     *
     * To mirror the content of a document using change events use the following
     * approach:
     * - start with the same initial content
     * - apply the 'textDocument/didChange' notifications in the order you
     *   receive them.
     * - apply the `TextDocumentContentChangeEvent`s in a single notification
     *   in the order you receive them.
     */
    std::vector<TextDocumentContentChangeEvent> contentChanges;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DidChangeTextDocumentParams,
                                   textDocument,
                                   contentChanges);

} // namespace lsp
