

-- Mock events

KeyEvent = {}

function KeyEvent:new(o)
    parent = parent or {}
    o = o or {}
    setmetatable(o, self)
    self.__index = self
    return o
end

function KeyEvent:match(e)
    print("matching ", e, "to", self.k)
    return self.k == e
end

function translateKeyEvent(k)
    return KeyEvent:new{k = k}
end


-- -------------------------------------

require "../data/mode"

isFunctionCalled = false
isVFunctionCalled = false

map{
    x = function() print("called") isFunctionCalled = true end
}

vmap{
    y = function() print("v called") isVFunctionCalled = true end
}


parent_mode:key_press{key = "x"}

assert(isFunctionCalled)
assert(not isVFunctionCalled)

visual_mode:key_press{key = "y"}

assert(isFunctionCalled)
assert(isVFunctionCalled)
