
#include "plugin/jsonformat.h"
#include "files/extensions.h"
#include "views/editor.h"
#include "json/json.h"

bool JsonFormat::format(Editor &editor) {
    auto path = editor.path();
    if (isJson(path)) {
        try {
            auto text = editor.buffer().text();

            auto json = Json(text);

            auto formatted = json.stringify();

            if (formatted != text) {
                editor.buffer().text(formatted);
            }
        }
        catch (Json::ParsingError &e) {
        }
        return true;
    }
    else {
        return false;
    }
}
