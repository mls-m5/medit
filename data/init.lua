
-- require "data/mode"
require(findScript("data/mode.lua"))

--- Default functions

function Editor.left(e)
    local editor = e.get_editor()
    editor.set_cursor(left(editor.get_cursor()))
end

function Editor.right(e)
    local editor = e.get_editor()
    editor.set_cursor(right(editor.get_cursor()))
end

-- Parent mode
map{
    F2 = goto_definition,
    F4 = switch_header,
    F5 = switch_header, -- test
}

--- Normal mode
nmap{
    Left = Editor.left,
    Right = Editor.right,
    Down = Editor.down,
    Up = Editor.up,
    h = Editor.left,
    l = Editor.right,
    j = Editor.down,
    k = Editor.up,

    a = function(e) Editor.right(e) Editor.insert_mode(e) end,
    b = function(e) Editor.end_(e) Editor.insert_mode(e) end,

    buffered = {
        dd = Editor.delete_line,
        dw = function(e) Editor.select_word(e) Editor.erase(e) end,
    }
}
