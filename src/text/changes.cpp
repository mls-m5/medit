#include "changes.h"
#include "core/coreenvironment.h"
#include "files/file.h"
#include "files/project.h"
#include "script/ienvironment.h"
#include "text/cursorrange.h"
#include "text/cursorrangeops.h"
#include <regex>

void Changes::serialize(std::ostream &out) const {
    for (const auto &fileChanges : changes) {
        for (const auto &change : fileChanges.changes) {
            out << fileChanges.file.string() << ":" << change.begin.y() + 1
                << ":" << change.begin.x() + 1 << "-" << change.end.y() + 1
                << ":" << change.end.x() + 1 << " " << change.newText
                << std::endl;
        }
    }
}

Changes::FileChanges &Changes::operator[](
    const std::filesystem::__cxx11::path &path) {
    auto it = std::find_if(
        changes.begin(), changes.end(), [&](const FileChanges &fileChanges) {
            return fileChanges.file == path;
        });
    if (it != changes.end()) {
        return *it;
    }
    else {
        changes.push_back({path, {}});
        return changes.back();
    }
}

void Changes::deserialize(std::istream &in) {
    changes.clear();
    std::string line;
    std::regex line_regex(R"((.+):(\d+):(\d+)-(\d+):(\d+)\s+(.+))");
    std::smatch line_match;

    while (std::getline(in, line)) {
        if (std::regex_match(line, line_match, line_regex)) {
            std::filesystem::path filePath = line_match[1].str();
            Position begin(std::stoi(line_match[3].str()) - 1,
                           std::stoi(line_match[2].str()) - 1);
            Position end(std::stoi(line_match[5].str()) - 1,
                         std::stoi(line_match[4].str()) - 1);

            std::string newText = line_match[6].str();

            FileChanges &fileChanges = (*this)[filePath];
            fileChanges.changes.push_back({begin, end, newText});
        }
    }
}

void Changes::sort() {
    for (auto &fileChanges : changes) {
        std::sort(fileChanges.changes.rbegin(),
                  fileChanges.changes.rend(),
                  [](const Change &a, const Change &b) {
                      if (a.begin.y() == b.begin.y()) {
                          return a.begin.x() < b.begin.x();
                      }
                      else {
                          return a.begin.y() < b.begin.y();
                      }
                  });
    }
}

void Changes::apply(IEnvironment &env) {
    // Handle open buffers

    auto &core = env.core();

    sort();

    for (auto it = changes.begin(); it != changes.end();) {
        auto &change = *it;

        if (auto buffer =
                core.find(env.project().settings().root / change.file)) {
            for (auto &c : change.changes) {
                c.apply(*buffer);
            }
            it = changes.erase(it);
        }
        else {
            ++it;
        }
    }

    // Handle files that is not opens
    for (auto it = changes.begin(); it != changes.end(); ++it) {
        auto file = File{env.project().settings().root / it->file};

        auto buffer = Buffer{};
        file.load(buffer);

        for (auto &c : it->changes) {
            c.apply(buffer);
        }

        file.save(buffer);
    }
}

void Changes::Change::apply(Buffer &buffer) const {
    auto range = CursorRange{buffer, begin, end};

    replace(range, newText);
}
