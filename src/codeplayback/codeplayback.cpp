
#include "files/config.h"
#include "files/file.h"
#include "sdlpp/image.hpp"
#include "syntax/basichighligting.h"
#include "text/bufferedit.h"
#include "text/cursorops.h"
#include "text/fstring.h"
#include "text/utf8char.h"
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
#include <limits>
#include <memory>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
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

auto testText4 = R"_(
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

// Struct that extract the numbers that is put before a line or after
struct FrameNumLineDescription {

    bool tryPrefix(const std::string fullLine) {
        auto line = fullLine;
        std::string beginStr = std::string{};
        while (!line.empty() && std::isdigit(line.front())) {
            beginStr += line.front();
            line.erase(0, 1);
        }

        if (line.empty() || (line.front() != '-' && line.front() != ':')) {
            begin = -1;
            end = std::numeric_limits<int>::max();
            this->line = fullLine;
            return false;
        }

        begin = std::stoi(beginStr);
        end = std::numeric_limits<int>::max();

        if (line.front() == ':') {
            this->line = line.substr(1);
            return true;
        }

        std::string endStr;
        // line.front must be '-'
        line.erase(0, 1); // remove '-'
        while (!line.empty() && std::isdigit(line.front())) {
            endStr += line.front();
            line.erase(0, 1);
        }

        if (line.empty() || line.front() != ':') {
            begin = -1;
            end = std::numeric_limits<int>::max();
            this->line = fullLine;
            return false;
        }

        line.erase(0, 1); // Remove ':'

        end = stoi(endStr);
        this->line = line;
        return true;
    }

    /// For formating sake i sometimes comment out some of the versions
    /// so that you could run clang-format on the file without it being messed
    /// up, but then you need to remove the comment
    /// This function only removes comments that has only spaces (indentation)
    /// before them
    std::string removeInitialComment(const std::string &line) {
        // Regex pattern to match lines that start with spaces followed by a
        // comment
        std::regex commentRegex(R"_(^(\s*)//\s*(.*))_");

        std::smatch match;
        if (std::regex_search(line, match, commentRegex)) {
            return match[1].str() + match[2].str();
        }
        return line;
    }

    bool tryComments(const std::string &line) {
        // Regex for range number
        std::regex rangeRegex(R"(\s*//\s*(\d+)-(\d+)\s*$)");
        // Regex for single number
        std::regex singleRegex(R"(\s*//\s*(\d+)\s*$)");

        std::smatch match;
        if (std::regex_search(line, match, rangeRegex)) {
            begin = std::stoi(match[1]);
            end = std::stoi(match[2]);
            this->line = removeInitialComment(line.substr(0, match.position()));
            return true;
        }
        else if (std::regex_search(line, match, singleRegex)) {
            begin = std::stoi(match[1]);
            this->line = removeInitialComment(line.substr(0, match.position()));
            return true;
        }
        this->line = line;
        return false;
    }

    FrameNumLineDescription(const std::string &fullLine) {
        if (tryPrefix(fullLine)) {
            return;
        }

        tryComments(fullLine);
    }

    std::string line; // The line without the line
    int begin = -1;
    int end = std::numeric_limits<int>::max();

    bool isInside(int i) const {
        return i >= begin && i < end;
    }

    operator bool() const {
        return begin >= 0;
    }

    bool hasEnd() const {
        return end < std::numeric_limits<int>::max();
    }
};

std::string extractSingleFrame(
    const std::vector<FrameNumLineDescription> &descriptions, int frameNum) {
    auto res = std::ostringstream{};

    for (auto &d : descriptions) {
        if (d.isInside(frameNum)) {
            res << d.line << "\n";
        }
    }

    return res.str();
}

std::vector<FrameNumLineDescription> streamToDescriptions(std::istream &ss) {
    auto descriptions = std::vector<FrameNumLineDescription>{};

    for (std::string line; std::getline(ss, line);) {
        auto descriptor = FrameNumLineDescription{line};
        descriptions.push_back(std::move(descriptor));
    }

    return descriptions;
}

std::vector<BufferEdit> descriptionsToBufferEdit(
    Buffer &buffer, const std::vector<FrameNumLineDescription> &descriptions) {
    int max = 0;
    for (auto &d : descriptions) {
        if (d.hasEnd()) {
            max = std::max(d.end, max);
        }
        max = std::max(d.begin + 1, max);
    }

    auto ret = std::vector<BufferEdit>{};
    auto lastFrame = std::string{};
    for (int i = 0; i < max; ++i) {
        auto newFrame = extractSingleFrame(descriptions, i);
        ret.push_back(createEdit(buffer, lastFrame, newFrame));
        lastFrame = newFrame;
    }

    return ret;
}

std::vector<BufferEdit> extractEditsFromString(Buffer &buffer, std::string in) {
    auto descriptions = [&] {
        auto ss = std::istringstream{std::move(in)};
        return streamToDescriptions(ss);
    }();

    return descriptionsToBufferEdit(buffer, descriptions);
}

std::vector<BufferEdit> loadEditsFromFile(Buffer &buffer,
                                          std::filesystem::path path) {
    auto descriptions = [&] {
        auto file = std::ifstream{path};
        return streamToDescriptions(file);
    }();

    return descriptionsToBufferEdit(buffer, descriptions);
}

constexpr auto helpStr = R"_(
usage
code_playback scriptfile.txt [options]

flags:
--help                 print this text
)_";

struct CodePlaybackSettings {
    std::filesystem::path scriptFile;

    CodePlaybackSettings(int argc, char **argv) {
        auto args = std::vector<std::string>{argv + 1, argv + argc};

        for (size_t i = 0; i < args.size(); ++i) {
            auto arg = args.at(i);

            if (arg == "--help") {
                std::cout << helpStr << std::endl;
                std::exit(0);
            }
            else {
                scriptFile = arg;
            }
        }
    }
};

int main(int argc, char *argv[]) {
    const auto settings = CodePlaybackSettings{argc, argv};

    auto screen = ScreenType{};
    screen.fontSize(30);

    screen.resize(60, 10);

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

    auto drawBufferEdit = [&](const BufferEdit edit) {
        for (auto &e : splitEdit(edit)) {
            auto cursor = apply(e);
            BasicHighlighting::highlightStatic(buffer);
            editor.cursor(cursor);
            editor.draw(screen);
            editor.updateCursor(screen);
            screen.refresh();
            videoDump.dump();
        }
    };

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
    outputPath.replace_extension(".mp4");

    videoDump.outputPath = outputPath;

    screen.cursorStyle(CursorStyle::Block);

    screen.cursorStyle(CursorStyle::Beam);

    for (auto &edit : edits) {
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
