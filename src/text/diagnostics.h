#pragma once

#include "position.h"
#include <string>
#include <vector>

enum class DiagnosticType {
    Note,
    Error,
    Warning,
    Breakpoint,      // Used by debugger
    RunningPosition, // Used by debugger
};

class Diagnostics {
public:
    struct Diagnostic {
        DiagnosticType type = DiagnosticType::Note;
        std::string source;
        std::string message;
        struct Range {
            Position begin;
            Position end;
        } range;
    };

    //! Clear all diagnosticsc from a single source
    void clearSource(std::string_view name);

    void push(Diagnostic data) {
        _list.push_back(std::move(data));
    }

    /// Clear earlier data from that source and replace it with new data
    void publish(std::string source, std::vector<Diagnostics::Diagnostic> data);

    // TODO: Handle this better, use
    Diagnostic *findLineDiagnostic(int line);

    const std::vector<Diagnostic> &list() const {
        return _list;
    }

private:
    std::vector<Diagnostic> _list;
};
