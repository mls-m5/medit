
#include "linuxterminalscreen.h"
#include "files/popenstream.h"
#include "syntax/color.h"
#include "text/fstring.h"
#include <sstream>
#include <termios.h>
#include <unistd.h>

namespace {

struct KeyTranslation {
    Key key;
    Utf8Char text{};
};

//! Keys that starts with ^
const auto keytranslations = std::map<int, KeyTranslation>{
    {27, {Key::Escape}},
    {127, {Key::Backspace}},
    {13, {Key::Text, "\n"}},
};

const auto escapeKeytranslations = std::map<int, KeyTranslation>{};

// std::string ttyString() {
//    std::ios::sync_with_stdio(false);
//    std::string ttyString;
//    POpenStream{"tty"} >> ttyString;
//    return ttyString;
//}

////! Check man stty for settings
// void setTtySetting(std::string setting) {
//    system(("stty -F " + ttyString() + " " + setting).c_str());
//}

termios originalTermiosSettings;

} // namespace

struct LinuxTerminalScreen::Style {
    Color foreground;
    Color background;
};

LinuxTerminalScreen::LinuxTerminalScreen() {
    _styles.resize(100);

    // For more info check, stty --help
    //    setTtySetting("icanon"); // Enable special characters
    //    setTtySetting("raw");
    //    setTtySetting("-echo");
    //    canvas.resize(width(), height());

    tcgetattr(STDIN_FILENO, &originalTermiosSettings);
    termios settings = originalTermiosSettings;

    //    settings.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    //    settings.c_oflag &= ~(OPOST);
    //    settings.c_cflag |= (CS8);
    //    settings.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    settings.c_iflag &= ~(ICRNL | IXON);
    // IXION is for ctrl+z and ctrl+q
    // ICRNL is for disabling ctrl+m -> \n and ctrl+j -> 13
    settings.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    // Isig prevents ctrl+c or ctrl+z from generating signals
    // IExten is for ctrl+v
    //    settings.c_cc[VMIN] = 0;  // Minimum characters needed to read (could
    //    be 0) settings.c_cc[VTIME] = 1; // Minimal time in tenths of seconds
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &settings);
}

LinuxTerminalScreen::~LinuxTerminalScreen() {
    //    system(("stty -F " + ttyString() + " sane").c_str());
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermiosSettings);
}

void LinuxTerminalScreen::draw(size_t x, size_t y, const FString &str) {
    // https://stackoverflow.com/questions/5947742/how-to-change-the-output-color-of-echo-in-linux/20983251#20983251
    //    system(("tput cup " + std::to_string(y) + " " +
    //    std::to_string(x)).c_str()); std::cout << std::string{str};
    //    std::cout.flush();

    //    canvas.draw(x, y, str);
    //    printf("\033[%d;%dH", static_cast<int>(y + 1), static_cast<int>(x +
    //    1));
    //    std::printf("%s", std::string{str}.c_str());

    std::ostringstream ss;

    auto setFgColor = [&ss](Color c) {
        ss << "\033[38;2;" << static_cast<int>(c.r()) << ";"
           << static_cast<int>(c.g()) << ";" << static_cast<int>(c.b()) << "m";
    };

    auto setBgColor = [&ss](Color c) {
        ss << "\033[48;2;" << static_cast<int>(c.r()) << ";"
           << static_cast<int>(c.g()) << ";" << static_cast<int>(c.b()) << "m";
    };

    auto prevF = FormatType{1};

    std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";

    for (auto &c : str) {
        if (c.f != prevF) {
            prevF = c.f;
            auto &style = _styles.at(c.f);
            setFgColor(style.foreground);
            setBgColor(style.background);
        }
        ss << &c.c.front();
    }

    std::cout << ss.str();
}

void LinuxTerminalScreen::refresh() {
    std::cout.flush();
}

void LinuxTerminalScreen::clear() {}

void LinuxTerminalScreen::cursor(size_t x, size_t y) {
    std::printf(
        "\033[%d;%dH", static_cast<int>(y + 1), static_cast<int>(x + 1));
    std::fflush(stdout);
}

// size_t LinuxTerminalScreen::x() const {
//     return 0;
// }

// size_t LinuxTerminalScreen::y() const {
//     return 0;
// }

// size_t LinuxTerminalScreen::width() const {
//     size_t width;
//     POpenStream{"tput cols"} >> width;
//     return width;
// }

// size_t LinuxTerminalScreen::height() const {
//     size_t height;
//     POpenStream{"tput lines"} >> height;
//     return height;
//     return 0;
// }

void LinuxTerminalScreen::title(std::string title) {}

Event LinuxTerminalScreen::getInput() {
    KeyEvent event;

    auto c = std::cin.get();

    event.symbol = c;

    if (c == 27) {
        auto c2 = std::cin.peek();
        if (c2 == '[') {
            std::cin.get();
            auto c3 = std::cin.get();
            if (auto f = escapeKeytranslations.find(c3);
                f != escapeKeytranslations.end()) {
                return KeyEvent{f->second.key, f->second.text};
            }
        }
    }
    if (auto f = keytranslations.find(c); f != keytranslations.end()) {
        return KeyEvent{f->second.key, f->second.text};
    }
    else if (c < 27) { // ctrl-characters
        // Note that ctrl+space -> 0
        return KeyEvent{Key::KeyCombination,
                        c == 0 ? ' ' : static_cast<char>(c + 'A' - 1),
                        Modifiers::Ctrl};
    }
    else if (c >= 28 && c <= 31) {
        // For some reason ctrl+shift+7 is 31
        return KeyEvent{Key::KeyCombination,
                        static_cast<char>('4' + c - 28),
                        Modifiers::Ctrl};
    }
    else {
        event.key = Key::Text;
    }
    return event;
}

size_t LinuxTerminalScreen::addStyle(const Color &foreground,
                                     const Color &background,
                                     size_t index) {
    if (index == std::numeric_limits<size_t>::max()) {
        ++_lastStyle;
        index = _lastStyle;
    }

    if (index + 1 > _styles.size()) {
        _styles.resize(index);
    }

    _styles.at(index) = {foreground, background};

    return index;
}

void LinuxTerminalScreen::cursorStyle(CursorStyle) {}
