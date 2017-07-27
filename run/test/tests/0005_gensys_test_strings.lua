--@Name Gensys string test
pegr.add_component('position.c', {
  x = {'f64', 17},
  y = {'f64', 19},
})

pegr.add_component('flavor.c', {
  flavor = {'str', 'sweet'},
  strength = {'f64', 1.0},
})

pegr.add_archetype('cube.at', {
  location = {
    __is = 'position.c',
  },
  taste = {
    __is = 'flavor.c',
    flavor = {'str', 'salty'},
  },
})

-- Proceed to the compilation stage
pegr.debug_stage_compile()

local arche = pegr.find_archetype('cube.at')
local ent = pegr.new_entity(arche)

local loc = ent.location
local tas = ent.taste

print('pod default check')
assert(loc.x == 17)
assert(loc.y == 19)

print('pod modification check')
loc.x = 23
loc.y = 29
assert(loc.x == 23)
assert(loc.y == 29)

print('string default check')
assert(tas.flavor == 'salty')

print('string modification check')
tas.flavor = 'sour'
assert(tas.flavor == 'sour')
