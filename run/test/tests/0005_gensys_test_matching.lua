--@Name Gensys component matching
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
local comp = pegr.find_component('flavor.c')

assert(arche)
assert(comp)
assert(ent)

print('arche', arche)
print('comp', comp)
print('ent', ent)

local cview = comp(ent)

assert(cview, "Failed to match component")

print('cview (match)', cview)
print('cview (named)', ent.taste)

assert(ent.taste == cview, "Unequal cviews!")

local aview = arche(ent)

assert(aview, "Failed to match archetype")

print('aview (match)', aview)
print('aview (basic)', ent)

assert(ent == aview, "Unequal aviews!")
