
#include "core/profiler.h"
#include "edit.h"
#include "files/config.h"
#include "files/file.h"
#include "modes/insertmode.h"
#include "playbacksettings.h"
#include "screen/cursorstyle.h"
#include "syntax/basichighligting.h"
#include "text/bufferedit.h"
#include "text/cursorops.h"
#include "videodump.h"
#include "views/editor.h"
#include "views/iwindow.h"
#include "views/view.h"
#include "views/window.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <istream>
#include <limits>
#include <memory>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// #ifdef __EMSCRIPTEN__

// #include "screen/htmlscreen.h"
// using ScreenType = HtmlScreen;

// #else

#include "screen/guiscreen.h"
using ScreenType = GuiScreen;

// #endif

struct PlaybackWindow : public Window {
public:
    PlaybackWindow(size_t width, size_t height)
        : View{nullptr, width, height}
        , Window{nullptr} {}

    bool keyPress(std::shared_ptr<IEnvironment> env) override {
        return true;
    }
    void updateCursor(IScreen &screen) const override {}
    bool mouseDown(int x, int y) override {
        return true;
    }

    void draw(IScreen &) override {}
};

int main(int argc, char *argv[]) {
    const auto settings = CodePlaybackSettings{argc, argv};

    if (settings.shouldEnableProfiling) {
        enableProfiling();
    }

    std::cout << "creating video from file " << settings.scriptFile
              << std::endl;
    std::cout << "output path " << settings.outputPath << std::endl;

    auto screen = ScreenType{};
    screen.fontSize(settings.fontSize);

    screen.resize(settings.viewportWidth, settings.viewportHeight);

    {
        auto palette = Palette{};
        palette.load(findConfig("data/oblivion-bg-black.json"));
        screen.palette(palette);
    }

    std::cout << "medit code playback" << std::endl;

    auto window = PlaybackWindow{screen.width(), screen.height()};

    auto editor = Editor{&window, std::make_shared<Buffer>()};
    editor.showLines(settings.hasLineNumbers);
    editor.mode(createInsertMode());

    auto &buffer = editor.buffer();
    /// Assigning for highlighting to be correct
    buffer.assignFile(std::make_unique<File>("/tmp/trashasthoeu.cpp"));

    editor.width(screen.width());
    editor.height(screen.height() - 1);

    bool isRunning = true;
    screen.subscribe([&isRunning](IScreen::EventListT list) {
        for (auto e : list) {
            if (auto k = std::get_if<KeyEvent>(&e)) {
                if (k->key == Key::Quit) {
                    isRunning = false;
                }
            }
        }
    });

    auto videoDump = VideoDump{screen};
    videoDump.fps = settings.fps;

    auto edits = std::vector<BufferEdit>{};

    if (!settings.scriptFile.empty()) {
        edits = loadEditsFromFile(buffer, settings.scriptFile);
    }

    std::cout << "\n =================== preview ==================\n";
    {
        int num = 0;
        for (auto &edit : edits) {
            std::cout << "\n=== preview edit " << num << " \n";
            std::cout << edit.to;
            ++num;
        }
    }
    std::cout << "\n ================ end preview =================\n"
              << std::endl;

    auto outputPath = std::filesystem::absolute(settings.scriptFile);

    if (settings.scriptFile.empty()) {
        outputPath = "output";
    }

    if (!settings.outputPath.empty()) {
        outputPath = std::filesystem::absolute(settings.outputPath);
    }

    outputPath.replace_extension(".mp4");

    videoDump.outputPath = outputPath;

    if (settings.cursor == "block") {
        screen.cursorStyle(CursorStyle::Block);
    }
    if (settings.cursor == "hidden" || settings.cursor == "off") {
        screen.cursorStyle(CursorStyle::Hidden);
    }
    else {
        screen.cursorStyle(CursorStyle::Beam);
    }

    auto count = edits.size();
    auto currentEdit = 0;

    auto drawBufferEdit = [&](const BufferEdit edit) {
        auto duration = ProfileDuration{};
        std::cout << "insert code:\n" << edit.to << std::endl;
        auto edits = splitEdit(edit);
        int num = 1;

        auto dump = [&] {
            editor.draw(screen);
            editor.updateCursor(screen);
            screen.refresh();
            videoDump.dump();
            ++num;
        };

        FString previousText;

        for (auto &e : edits) {
            if (num % 10 == 0) {
                std::cout << currentEdit << "/" << count << ": subframe " << num
                          << "/" << edits.size() << std::endl;
            }
            const auto oldCursor = editor.cursor();

            auto dist = static_cast<ptrdiff_t>(e.position.y()) -
                        static_cast<ptrdiff_t>(editor.cursor().y());
            // Animate motion if the jump is to large
            if (std::abs(dist) > 3) {
                screen.cursorStyle(CursorStyle::Block);

                const auto cursor = e.position;
                ptrdiff_t sign = dist > 0 ? 1 : -1;

                for (auto y = static_cast<ptrdiff_t>(oldCursor.y());
                     y != static_cast<ptrdiff_t>(cursor.y());
                     y += sign) {
                    editor.cursor(Cursor{
                        cursor.buffer(), cursor.x(), static_cast<size_t>(y)});
                    dump();
                }
                screen.cursorStyle(CursorStyle::Beam);
            }

            const auto cursor = apply(e);
            editor.cursor(cursor);

            /// Skip taking so long time for indentations
            if (e.to == " " && (previousText == " " || previousText == "\n") &&
                content(Cursor{e.position.buffer(), 0, e.position.y()}) ==
                    ' ') {
                previousText = e.to;
                continue;
            }

            BasicHighlighting::highlightStatic(buffer);
            dump();

            previousText = e.to;
        }
    };

    for (auto &edit : edits) {
        std::cout << currentEdit << "/" << count << std::endl;
        ++currentEdit;

        if (currentEdit < settings.startFrameNumber ||
            currentEdit > settings.stopFrameNumber) {
            continue;
        }

        drawBufferEdit(edit);
        if (!isRunning) {
            break;
        }

        videoDump.finish();
    }

    screen.unsubscribe();

    auto returnCode = videoDump.finish();

    return returnCode;
}
