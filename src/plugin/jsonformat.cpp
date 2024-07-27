
#include "plugin/jsonformat.h"
#include "files/extensions.h"
#include "text/buffer.h"
#include "views/editor.h"
#include "json/json.h"
#include <iostream>

bool JsonFormat::format(Editor &editor) {
    auto path = editor.path();
    if (!isJson(path)) {
        return false;
    }
    try {
        auto text = editor.buffer().text();

        auto json = Json::Parse(text);

        auto formatted = json.stringify();

        if (formatted != text) {
            editor.buffer().text(formatted);
        }
    }
    catch (Json::ParsingError &e) {
        auto cursor =
            editor.buffer().cursor({e.position.col - 1, e.position.line - 1});
        editor.cursor(cursor);
        std::cerr << e.what() << std::endl;
    }
    catch (std::exception &e) {
    }
    return true;
}
