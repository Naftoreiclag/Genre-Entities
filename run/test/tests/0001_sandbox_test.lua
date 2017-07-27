--@Name Simple sandbox test

local unsafe = {
  'collectgarbage',
  'dofile',
  'dump',
  'load',
  'loadfile',
  'loadstring',
}

local accessed = {}

for _, name in ipairs(unsafe) do
  if _G[name] ~= nil then 
    table.insert(accessed, name)
  end
end

if #accessed > 0 then
  error('Accessed: ' .. table.concat(accessed, ', '))
end
