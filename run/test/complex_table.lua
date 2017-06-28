local inside = {
  a = "apple",
  b = "banana",
  c = "cherry",
  d = "durian",
}

local function func()
  return "return"
end

return {
  [1] = "one",
  [2] = "two",
  [3] = "three",
  "4" = inside,
  inside = func,
}
