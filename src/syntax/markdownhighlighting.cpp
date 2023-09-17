#include "markdownhighlighting.h"
#include "files/extensions.h"
#include "syntax/palette.h"
#include "text/cursorrange.h"
#include "text/cursorrangeops.h"
#include "text/formattype.h"
#include "text/fstring.h"
#include <optional>

namespace {

std::optional<CursorRange> match(Buffer &buffer, size_t l, char a, char b) {
    auto line = buffer.lineAt(l);
    auto f1 = line.find(a);

    if (f1 == FString::npos) {
        return std::nullopt;
    }

    auto f2 = line.find(b, f1 + 1);

    if (f2 == FString::npos) {
        return std::nullopt;
    }

    return CursorRange{buffer, {f1, l}, {f2, l}};
}

void tryFormat(Buffer &buffer, char a, char b, FormatType f) {
    auto &lines = buffer.lines();
    for (size_t i = 0; i < lines.size(); ++i) {
        auto line = lines.at(i);

        if (auto m = match(buffer, i, a, b)) {
            format(*m, f);
        }
    }
}

size_t tryFormatLinkLine(Buffer &buffer, size_t l, size_t start) {
    auto line = buffer.lineAt(l);
    auto f1 = line.find('[', start);

    if (f1 == FString::npos) {
        return FString::npos;
    }

    auto f2 = line.find(']', f1 + 1);

    if (f2 == FString::npos) {
        return f1 + 1;
    }

    if (f2 + 1 >= line.size()) {
        return f2 + 1;
    }

    if (line.at(f2 + 1) != '(') {
        return f2 + 2;
    }

    auto f3 = line.find(')', f2 + 2);

    if (f3 == FString::npos) {
        return f3 + 1;
    }

    format(CursorRange{buffer, {f1, l}, {f3 + 1, l}}, Palette::identifier);

    return f3 + 1;
}

void tryFormatLink(Buffer &buffer) {
    auto &lines = buffer.lines();
    for (size_t i = 0; i < lines.size(); ++i) {
        auto line = lines.at(i);
        for (size_t start = 0;
             (start = tryFormatLinkLine(buffer, i, start)) != FString::npos;) {
        }
    }
}

void formatCodeContent(Buffer &buffer, size_t begin, size_t end) {
    auto range = CursorRange{buffer, {0, begin}, {1000, end}};
    format(range, Palette::comment);
}

void formatCodeBlocks(Buffer &buffer) {
    auto lines = buffer.lines();
    const auto codeStart = FString{"```"};
    for (size_t i = 0; i < lines.size(); ++i) {
        auto &line = lines.at(i);
        if (line.startsWith(codeStart)) {
            for (size_t j = i + 1; j < lines.size(); ++j) {
                auto &line2 = lines.at(j);

                if (line2.startsWith(codeStart)) {
                    formatCodeContent(buffer, i, j + 1);
                    i = j + 1;
                    break;
                }
            }
        }
    }
}

} // namespace

MarkdownHighlighting::MarkdownHighlighting() = default;

MarkdownHighlighting::~MarkdownHighlighting() = default;

bool MarkdownHighlighting::shouldEnable(std::filesystem::path path) {
    return isMarkdown(path);
}

bool MarkdownHighlighting::highlight(Buffer &buffer) {
    if (!shouldEnable(buffer.path())) {
        return false;
    }

    auto &lines = buffer.lines();
    for (size_t i = 0; i < lines.size(); ++i) {
        auto line = lines.at(i);
        if (line.empty()) {
            continue;
        }
        if (line.front() == '#') {
            format(CursorRange(buffer, {0, i}, {1000, i}), Palette::identifier);
        }
    }

    tryFormat(buffer, '`', '`', Palette::comment);
    tryFormatLink(buffer);

    buffer.emitChangeSignal();

    formatCodeBlocks(buffer);

    return true;
}
