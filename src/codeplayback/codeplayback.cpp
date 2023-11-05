
#include "core/profiler.h"
#include "edit.h"
#include "files/config.h"
#include "files/file.h"
#include "playbacksettings.h"
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

using namespace std::literals;

constexpr auto testText = R"_(
int main() {
    std::cout << "hello, world\n";
}
)_"sv;

constexpr auto testText2 = R"_(
int main() {
    std::cout << "hello there\n";
}
)_"sv;

constexpr auto testText3 = R"_(
int main() {
    std::cout << "hello there\n";
    std::cout << "zup\n";
}
)_"sv;

constexpr auto testText4 = R"_(
int main() {
1-2:    std::cout << "hello, world\n";
2:    std::cout << "hello there\n";
3:    std::cout << "zup\n";
}
)_"sv;

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
    editor.showLines(true);

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
    else if (false) {
        edits.push_back(createEdit(buffer, "", testText));
        edits.push_back(createEdit(buffer, testText, testText2));
        edits.push_back(createEdit(buffer, testText2, testText3));
        edits.push_back(createEdit(buffer, testText3, ""));
    }
    else {
        edits = extractEditsFromString(buffer, std::string{testText4});
    }

    auto outputPath = std::filesystem::absolute(settings.scriptFile);

    if (settings.scriptFile.empty()) {
        outputPath = "output";
    }

    if (!settings.outputPath.empty()) {
        outputPath = std::filesystem::absolute(settings.outputPath);
    }

    outputPath.replace_extension(".mp4");

    videoDump.outputPath = outputPath;

    screen.cursorStyle(CursorStyle::Block);

    screen.cursorStyle(CursorStyle::Beam);

    auto count = edits.size();
    auto currentEdit = 0;

    auto drawBufferEdit = [&](const BufferEdit edit) {
        auto duration = ProfileDuration{};
        std::cout << "insert code " << edit.to << std::endl;
        auto edits = splitEdit(edit);
        int num = 1;
        for (auto &e : edits) {
            if (num % 10 == 0) {
                std::cout << currentEdit << "/" << count << ": subframe " << num
                          << "/" << edits.size() << std::endl;
            }
            auto cursor = apply(e);
            BasicHighlighting::highlightStatic(buffer);
            editor.cursor(cursor);
            editor.draw(screen);
            editor.updateCursor(screen);
            screen.refresh();
            videoDump.dump();
            ++num;
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
