#pragma once

#include "text/bufferedit.h"
#include <fstream>
#include <regex>
#include <string>

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
    enum Type {
        Normal,
        BlockStart,
        BlockEnd,
    };

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
            auto res = match[1].str() + match[2].str();

            auto m2 = match[2].str();
            if (!m2.empty() && m2.front() == '$') {
                m2.erase(0, 1);
                return match[1].str() + m2;
            }

            return res;
        }
        return line;
    }

    void useMatch(const std::smatch &match, int num, const std::string &line) {
        begin = std::stoi(match[1]);
        if (num > 1) {
            end = std::stoi(match[2]);
        }
        this->line = removeInitialComment(line.substr(0, match.position()));
    }

    bool tryComments(const std::string &line) {
        std::smatch match;
        {
            // Regex for range number
            static const std::regex rangeRegex(R"(\s*//\s*(\d+)-(\d+)\s*$)");
            static const std::regex singleRegex(R"(\s*//\s*(\d+)\s*$)");

            if (std::regex_search(line, match, rangeRegex)) {
                //            begin = std::stoi(match[1]);
                //            end = std::stoi(match[2]);
                //            this->line = removeInitialComment(line.substr(0,
                //            match.position()));
                useMatch(match, 2, line);
                return true;
            }
            else if (std::regex_search(line, match, singleRegex)) {
                //            begin = std::stoi(match[1]);
                //            this->line = removeInitialComment(line.substr(0,
                //            match.position()));
                useMatch(match, 1, line);
                return true;
            }
        }

        {
            static const std::regex blockBeginRangeRegex(
                R"(\s*//\s*block (\d+)-(\d+)\s*$)");
            static const std::regex blockBeginSingleRegex(
                R"(\s*//\s*block (\d+)\s*$)");

            if (std::regex_search(line, match, blockBeginRangeRegex)) {
                useMatch(match, 2, line);
                type = Type::BlockStart;
                return true;
            }
            else if (std::regex_search(line, match, blockBeginSingleRegex)) {
                useMatch(match, 1, line);
                type = Type::BlockStart;
                return true;
            }
        }

        {
            static const std::regex blockEndRangeRegex(
                R"(\s*//\s*end (\d+)-(\d+)\s*$)");
            static const std::regex blockEndSingleRegex(
                R"(\s*//\s*end (\d+)\s*$)");

            if (std::regex_search(line, match, blockEndRangeRegex)) {
                useMatch(match, 2, line);
                type = Type::BlockEnd;
                return true;
            }
            else if (std::regex_search(line, match, blockEndSingleRegex)) {
                useMatch(match, 1, line);
                type = Type::BlockEnd;
                return true;
            }
            else if (line.find("// end") != std::string::npos) {
                type = Type::BlockEnd;
                return true;
            }
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

    /// Used for blocks and similar when the information is already known
    FrameNumLineDescription(std::string text, int begin, int end)
        : line{text}
        , begin{begin}
        , end{end} {}

    std::string line; // The line without the line
    int begin = -1;
    int end = std::numeric_limits<int>::max();
    Type type = Type::Normal;

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

    bool removeBlock = false;

    for (auto &d : descriptions) {
        if (d.type == FrameNumLineDescription::BlockStart) {
            if (!d.isInside(frameNum)) {
                removeBlock = true;
            }
        }

        if (removeBlock) {
            if (d.type != FrameNumLineDescription::BlockEnd) {
                continue;
            }
            removeBlock = false;
            continue;
        }

        if (d.type != FrameNumLineDescription::Normal) {
            continue;
        }

        if (d.isInside(frameNum)) {
            res << d.line << "\n";
        }
    }

    return res.str();
}

std::vector<FrameNumLineDescription> streamToDescriptions(std::istream &ss) {
    auto descriptions = std::vector<FrameNumLineDescription>{};

    int isHead = 2; // 2 = Comments at beginning of file, 1 = spaces after this
    for (std::string line; std::getline(ss, line);) {
        if (isHead == 2) {
            if (line.rfind("//", 0) == 0) {
                continue;
            }
            isHead = 1;
        }
        if (isHead == 1) {
            if (line.empty()) {
                continue;
            }

            isHead = 0;
        }
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
