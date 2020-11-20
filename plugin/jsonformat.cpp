
#include "plugin/jsonformat.h"
#include "files/extensions.h"
#include "text/buffer.h"
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
        catch (std::exception &e) {
        }
        return true;
    }
    else {
        return false;
    }
}
