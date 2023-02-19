
Mode = {
    key_map = {}
}

function Mode:new(o, parent)
    parent = parent or {}
    o = o or {}
    setmetatable(o, self)
    self.__index = self
    o.parent = self
    return o
end

function createKeyMap(name, f)
    return {
        key = translateKeyEvent(name),
        f = f,
    }
end

function Mode:map(args)
    for i,v in pairs(args) do
        self.key_map[i] = createKeyMap(i, v)
    end
end

function Mode:key_press(e)
    for i,v in pairs(self.key_map) do
        if (i:match(e.key)) then
            v:f(e)
            break
        end
    end
end

--- Standard modes

parent_mode = Mode:new{}
normal_mode = Mode:new{parent_mode}
insert_mode = Mode:new{parent_mode}
visual_mode = Mode:new{parent_mode}

function map(args)
    parent_mode:map(args)
end

function nmap(args)
    normal_mode:map(args)
end

function vmap(args)
    visual_mode:map(args)
end

function imap(args)
    insert_mode:map(args)
end

print("mode");
