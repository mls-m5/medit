
#include "files/config.h"
#include "files/file.h"
#include "syntax/basichighligting.h"
#include "text/bufferedit.h"
#include "text/cursorops.h"
#include "text/fstring.h"
#include "text/utf8char.h"
#include "views/editor.h"
#include "views/iwindow.h"
#include "views/view.h"
#include "views/window.h"
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#ifdef __EMSCRIPTEN__

#include "screen/htmlscreen.h"
using ScreenType = HtmlScreen;

#else

#include "screen/guiscreen.h"
using ScreenType = GuiScreen;

#endif

using namespace std::literals;

auto testText = R"_(
int main() {
    std::cout << "hello, world\n";
}
)_"sv;

auto testText2 = R"_(
int main() {
    std::cout << "hello there\n";
}
)_"sv;

auto testText3 = R"_(
int main() {
    std::cout << "hello there\n";
    std::cout << "zup\n";
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

BufferEdit createEdit(Buffer &buffer, FString textA, FString textB) {
    auto edit = BufferEdit{
        .from = textA,
        .to = textB,
        .position = {buffer, 0, 0},
    };
    edit.trim();
    return edit;
}

/// Creates smaller edits (single characters) that looks more like written text
std::vector<BufferEdit> splitEdit(const BufferEdit &old) {
    auto ret = std::vector<BufferEdit>{};

    {
        // Note that the first edit just removes the text
        ret.push_back(BufferEdit{
            .from = old.from,
            .to = {},
            .position = old.position,
        });
    }

    auto position = old.position;
    for (auto c : old.to) {
        ret.push_back({
            .from = "",
            .to = FString{1, c},
            .position = position,
        });

        if (c == '\n') {
            position.x(0);
            position.y(position.y() + 1);
        }
        else {
            position.x(position.x() + 1);
        }
    }

    return ret;
}

int main(int argc, char *argv[]) {
    auto screen = ScreenType{};

    {
        auto palette = Palette{};
        palette.load(findConfig("data/oblivion.json"));
        screen.palette(palette);
    }

    std::cout << "hello\n";

    auto window = PlaybackWindow{screen.width(), screen.height()};

    auto editor = Editor{&window, std::make_shared<Buffer>()};
    editor.showLines(true);

    auto &buffer = editor.buffer();
    buffer.assignFile(std::make_unique<File>("/tmp/trashasthoeu.cpp"));

    editor.width(screen.width());
    editor.height(screen.height() - 1);

    auto wasAlpha = false;

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

    auto insertCharacter = [&](Position position, Utf8Char c) {
        screen.cursorStyle(CursorStyle::Beam);
        insert(c, {buffer, position});
        editor.draw(screen);
        editor.cursor({buffer, position += c});
        editor.updateCursor(screen);
        screen.refresh();
        auto a = isalpha(c);
        if (!a && a != wasAlpha) {
            std::this_thread::sleep_for(100ms);
        }
        else {
            std::this_thread::sleep_for(20ms);
        }
        wasAlpha = a;
    };

    auto drawBufferEdit = [&](const BufferEdit edit) {
        for (auto &e : splitEdit(edit)) {
            auto cursor = apply(e);
            BasicHighlighting::highlightStatic(buffer);
            editor.cursor(cursor);
            editor.draw(screen);
            editor.updateCursor(screen);
            screen.refresh();
            std::this_thread::sleep_for(20ms);
        }
    };

    for (; isRunning;) {
        screen.cursorStyle(CursorStyle::Block);

        //        for (auto c : testText) {
        //            insertCharacter(Position(buffer.end()), c);
        //        }

        screen.cursorStyle(CursorStyle::Beam);

        drawBufferEdit(createEdit(buffer, "", testText));
        std::this_thread::sleep_for(400ms);
        drawBufferEdit(createEdit(buffer, testText, testText2));
        std::this_thread::sleep_for(400ms);
        drawBufferEdit(createEdit(buffer, testText2, testText3));
        std::this_thread::sleep_for(1000ms);
        drawBufferEdit(createEdit(buffer, testText3, ""));

        screen.cursorStyle(CursorStyle::Block);
        screen.refresh();
    }

    screen.unsubscribe();

    return 0;
}
