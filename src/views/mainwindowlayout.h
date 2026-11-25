#pragma once

#include <algorithm>
#include <cstddef>

// Character-cell geometry shared by the GUI and terminal window.
struct MainWindowLayout {
    size_t sidebarWidth = 0; // Includes its separator.
    size_t editorWidth = 0;
    size_t editorHeight = 0;
    size_t consoleRows = 0; // Includes the separator, excludes the status bar.

    static MainWindowLayout calculate(size_t width,
                                      size_t height,
                                      size_t editorCount,
                                      bool sidebarRequested,
                                      bool showConsole,
                                      size_t split) {
        MainWindowLayout result;
        const auto contentHeight = height > 0 ? height - 1 : 0;
        // Leave at least one editor row when the console is visible.
        if (showConsole && contentHeight > 1 && split > 1) {
            result.consoleRows = std::min(split - 1, contentHeight - 1);
        }
        result.editorHeight = contentHeight - result.consoleRows;
        constexpr size_t sidebarWidth = 25;
        constexpr size_t minimumEditorWidth = 8;
        if (sidebarRequested && result.editorHeight > 0 && editorCount > 0 &&
            width >= sidebarWidth &&
            (width - sidebarWidth) / editorCount >= minimumEditorWidth) {
            result.sidebarWidth = sidebarWidth;
        }
        result.editorWidth = width - result.sidebarWidth;
        return result;
    }

    size_t editorX(size_t index, size_t count) const {
        return sidebarWidth + index * (editorWidth / count) +
               std::min(index, editorWidth % count);
    }

    size_t editorColumns(size_t index, size_t count) const {
        return editorWidth / count + (index < editorWidth % count ? 1 : 0);
    }
};
