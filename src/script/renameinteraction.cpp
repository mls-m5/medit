#include "renameinteraction.h"
#include "ienvironment.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include <memory>
#include <sstream>
#include <vector>

namespace {

struct Interaction {
    std::string op;
    std::vector<std::pair<std::string, std::string>> values;

    bool valid = true;

    /// Create text form
    void serialize(std::ostream &os) {
        if (!valid) {
            return;
        }

        os << op << "\n";
        for (const auto &[key, value] : values) {
            os << key << ": " << value << "\n";
        }
    }

    /// Parse text form
    void deserialize(std::istream &is) {
        valid = false;
        values.clear();

        std::string line;
        if (!std::getline(is, line)) {
            valid = false;
            return;
        }

        {
            if (size_t commentStart = line.find('#');
                commentStart != std::string::npos) {
                line = line.substr(0, commentStart);
            }
            line.erase(std::remove_if(line.begin(), line.end(), ::isspace),
                       line.end());
        }

        op = line;

        while (std::getline(is, line)) {
            if (auto commentStart = line.find('#');
                commentStart != std::string::npos) {
                line = line.substr(0, commentStart);
            }

            size_t colonPos = line.find(':');
            if (colonPos == std::string::npos) {
                continue;
            }

            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);

            // TODO: This removes all spaces. This might be okay for renaming
            // but might need refactorization later on
            key.erase(std::remove_if(key.begin(), key.end(), ::isspace),
                      key.end());
            value.erase(std::remove_if(value.begin(), value.end(), ::isspace),
                        value.end());
            values.emplace_back(key, value);
        }

        valid = !op.empty() && !values.empty();
    }
};

} // namespace

void beginRenameInteraction(std::shared_ptr<IEnvironment> env) {
    auto &e = env->editor();
    auto cursor = e.cursor();
    auto range = CursorRange{::wordBegin(cursor), ::right(::wordEnd(cursor))};
    auto old = content(range);

    //    auto replace = [](std::string str, std::string_view c, std::string
    //    insert) {
    //        auto position = str.find(c);
    //        str.replace(position, c.size(), insert);
    //        return str;
    //    };

    //    std::string str = R"(
    // rename
    // from: xxx_placeholder
    // to: yyy_placeholder
    // location: zzz_placeholder
    // # Press return to accept, esc to abort
    //)";

    auto fileStr = std::string{};

    //    str = replace(str, "xxx_placeholder", old);
    //    str = replace(str, "yyy_placeholder", old);
    if (auto f = e.file()) {
        // TODO: Handle for buffers that is not saved
        //        str = replace(str,
        //                      "zzz_placeholder",
        fileStr = e.file()->path().string() + ":" + std::to_string(cursor.y()) +
                  "," + std::to_string(cursor.x());
    }

    auto i = Interaction{"rename",
                         {
                             {"from", old},
                             {"to", old},
                             {"to", fileStr},
                         }};

    auto ss = std::ostringstream{};

    i.serialize(ss);

    insert(env->console().buffer().end(), ss.str());
    insert(env->console().buffer().end(),
           "\n#rename symbol is not implemented yet...");
    env->console().cursor(env->console().buffer().end());
    env->showConsole(true);
}
