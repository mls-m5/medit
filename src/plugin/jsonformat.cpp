
#include "plugin/jsonformat.h"
#include "core/plugins.h"
#include "files/extensions.h"
#include "text/buffer.h"
#include "views/editor.h"
#include "json/json.h"

bool JsonFormat::format(Editor &editor) {
    auto path = editor.path();
    if (isJson(path)) {
        try {
            auto text = editor.buffer().text();

            auto json = Json::Parse(text);

            auto formatted = json.stringify();

            if (formatted != text) {
                editor.buffer().text(formatted);
            }
        }
        catch (Json::ParsingError &e) {
            auto cursor = editor.buffer().cursor(
                {e.position.col - 1, e.position.line - 1});
            editor.cursor(cursor);
        }
        catch (std::exception &e) {
        }
        return true;
    }
    else {
        return false;
    }
}
