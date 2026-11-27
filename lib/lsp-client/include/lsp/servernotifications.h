#pragma once

#include "lsptypes.h"

namespace lsp {

enum class DiagnosticSeverity {
    /// The it's often technically optional some times
    Undefined = 0,

    /**
     * Reports an error.
     */
    Error = 1,
    /**
     * Reports a warning.
     */
    Warning = 2,
    /**
     * Reports an information.
     */
    Information = 3,
    /**
     * Reports a hint.
     */
    Hint = 4,
};

// NLOHMANN_JSON_SERIALIZE_ENUM(DiagnosticSeverity);

/**
 * The diagnostic tags.
 *
 * @since 3.15.0
 */
enum class DiagnosticTag {
    /// The it's often technically optional some times
    Undefined = 0,

    /**
     * Unused or unnecessary code.
     *
     * Clients are allowed to render diagnostics with this tag faded out
     * instead of having an error squiggle.
     */
    Unnecessary = 1,
    /**
     * Deprecated or obsolete code.
     *
     * Clients are allowed to rendered diagnostics with this tag strike through.
     */
    Deprecated = 2,
};

/**
 * Represents a related message and source code location for a diagnostic.
 * This should be used to point to code locations that cause or are related to
 * a diagnostics, e.g when duplicating a symbol in a scope.
 */
struct DiagnosticRelatedInformation {
    /**
     * The location of this related diagnostic information.
     */
    Location location;

    /**
     * The message of this related diagnostic information.
     */
    std::string message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DiagnosticRelatedInformation,
                                   location,
                                   message);

/**
 * Structure to capture a description for an error code.
 *
 * @since 3.16.0
 */
struct CodeDescription {
    /**
     * An URI to open with more information about the diagnostic error.
     */
    URI href;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CodeDescription, href);

struct Diagnostic {
    /**
     * The range at which the message applies.
     */
    Range range;

    /**
     * The diagnostic's severity. Can be omitted. If omitted it is up to the
     * client to interpret diagnostics as error, warning, info or hint.
     */
    DiagnosticSeverity severity = DiagnosticSeverity::Undefined;

    /**
     * The diagnostic's code, which might appear in the user interface.
     */
    //    code ?: integer | string;
    std::string code; // TODO: I just ignore integer values for simplicity

    /**
     * An optional property to describe the error code.
     *
     * @since 3.16.0
     */
    CodeDescription codeDescription;

    /**
     * A human-readable string describing the source of this
     * diagnostic, e.g. 'typescript' or 'super lint'.
     */
    std::string source;

    /**
     * The diagnostic's message.
     */
    std::string message;

    /**
     * Additional metadata about the diagnostic.
     *
     * @since 3.15.0
     */
    std::vector<DiagnosticTag> tags;

    /**
     * An array of related diagnostic information, e.g. when symbol-names within
     * a scope collide all definitions can be marked via this property.
     */
    // Todo: Optional
    std::vector<DiagnosticRelatedInformation> relatedInformation;

    /**
     * A data entry field that is preserved between a
     * `textDocument/publishDiagnostics` notification and
     * `textDocument/codeAction` request.
     *
     * @since 3.16.0
     */
    nlohmann::json data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Diagnostic,
                                                range,
                                                severity,
                                                code,
                                                codeDescription,
                                                source,
                                                message,
                                                tags,
                                                relatedInformation,
                                                data);

struct PublishDiagnosticsParams {
    static constexpr std::string_view method =
        "textDocument/publishDiagnostics";
    /**
     * The URI for which diagnostic information is reported.
     */
    DocumentUri uri;

    /**
     * Optional the version number of the document the diagnostics are published
     * for.
     *
     * @since 3.15.0
     */
    int version = -1;

    /**
     * An array of diagnostic information items.
     */
    std::vector<Diagnostic> diagnostics;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PublishDiagnosticsParams,
                                                uri,
                                                version,
                                                diagnostics);

enum class MessageType {
    Error = 1,   // An error message
    Warning = 2, // A warning message
    Info = 3,    // An information message
    Log = 4,     // A log message
    Debug = 5    // A debug message
};

std::string getMessageTypeName(MessageType messageType) {
    switch (messageType) {
    case MessageType::Error:
        return "Error";
    case MessageType::Warning:
        return "Warning";
    case MessageType::Info:
        return "Info";
    case MessageType::Log:
        return "Log";
    case MessageType::Debug:
        return "Debug";
    default:
        return "Unrecognized Message Type";
    }
}

struct ShowMessageParams {
    static constexpr std::string_view method = "window/showMessage";

    /**
     * The message type. See {@link MessageType}.
     */
    MessageType type = MessageType::Log;

    /**
     * The actual message.
     */
    std::string message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ShowMessageParams,
                                                type,
                                                message);

} // namespace lsp
