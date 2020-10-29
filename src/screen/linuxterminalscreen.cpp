
#include "linuxterminalscreen.h"
#include "files/popenstream.h"
#include "text/fstring.h"

namespace {

struct Canvas {
    std::vector<FChar> data;
    size_t width = 0;

    FChar &at(size_t x, size_t y) {
        return data.at(y * width + x);
    }

    void draw(size_t x, size_t y, const FString &str) {
        for (size_t tx = x; (tx + x) < width && tx < str.size(); ++tx) {
            at(tx + x, y) = str.at(tx - x);
        }
    }

    void resize(size_t w, size_t h) {
        width = w;
        data.resize(w * h);
    }

    void clear() {
        for (auto &c : data) {
            c = {" "};
        }
    }

    void refresh() {
        system("tput cup 0 0");
        for (auto &c : data) {
            std::cout << std::string{c};
        }
        std::cout.flush();
    }
};

std::string ttyString() {
    std::ios::sync_with_stdio(false);
    std::string ttyString;
    POpenStream{"tty"} >> ttyString;
    return ttyString;
}

//! Check man stty for settings
void setTtySetting(std::string setting) {
    system(("stty -F " + ttyString() + " " + setting).c_str());
}

Canvas canvas;

} // namespace

LinuxTerminalScreen::LinuxTerminalScreen() {
    //    system(("stty -F " + ttyString() + " raw").c_str());
    setTtySetting("raw");
    setTtySetting("-echo");
    canvas.resize(width(), height());
}

LinuxTerminalScreen::~LinuxTerminalScreen() {
    system(("stty -F " + ttyString() + " sane").c_str());
}

void LinuxTerminalScreen::draw(size_t x, size_t y, const FString &str) {
    // https://stackoverflow.com/questions/5947742/how-to-change-the-output-color-of-echo-in-linux/20983251#20983251
    //    system(("tput cup " + std::to_string(y) + " " +
    //    std::to_string(x)).c_str()); std::cout << std::string{str};
    //    std::cout.flush();
    canvas.draw(x, y, str);
}

void LinuxTerminalScreen::refresh() {
    canvas.refresh();
}

void LinuxTerminalScreen::clear() {
    canvas.clear();
}

void LinuxTerminalScreen::cursor(size_t x, size_t y) {}

size_t LinuxTerminalScreen::x() const {
    return 0;
}

size_t LinuxTerminalScreen::y() const {
    return 0;
}

size_t LinuxTerminalScreen::width() const {
    size_t width;
    POpenStream{"tput cols"} >> width;
    return width;
}

size_t LinuxTerminalScreen::height() const {
    size_t height;
    POpenStream{"tput lines"} >> height;
    return height;
    return 0;
}

KeyEvent LinuxTerminalScreen::getInput() {
    KeyEvent event;

    event.symbol = std::cin.get();
    event.key = Key::Text;

    return event;
}
