local tb1 = {}
local tb2 = {}
local tb3 = {}
local tb4 = {}
local tb5 = {}

local str = "bottom"

setmetatable(tb1, {__tostring = function() return str end})
setmetatable(tb2, {__tostring = function() return tb1 end})
setmetatable(tb3, {__tostring = function() return tb2 end})
setmetatable(tb4, {__tostring = function() return tb3 end})
setmetatable(tb5, {__tostring = function() return tb4 end})

return tb5

