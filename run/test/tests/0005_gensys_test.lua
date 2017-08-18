--@Name Basic Gensys test

pegr.add_component('position.c', {
  x = {'f64', 315},
  y = {'f64', 630},
})

pegr.add_archetype('simplex.at', {
  position = {
    __is = 'position.c',
  }
})

-- Proceed to the compilation stage
pegr.debug_stage_compile()

local arche = pegr.find_archetype('simplex.at')
local ent = pegr.new_entity(arche)

do
  local other_ent = pegr.new_entity(arche)
  assert(ent.__id ~= other_ent.__id, 'entity IDs must be unique!')
end

assert(ent.__arche == arche, 'Mismatched archetypes')
assert(not ent.__killed, 'Entity already killed')
assert(not ent.__alive, 'Entity already alive')
assert(not ent.__spawned, 'Entity already spawned')

assert(ent.position.x == 315, 'Expected 315, got ' .. tostring(ent.position.x))

ent.position.x = 500

assert(ent.position.x == 500, 'Expected 500, got ' .. tostring(ent.position.x))

ent.position.x = 0
for i = 1,100 do
  ent.position.x = ent.position.x + 1
end

assert(ent.position.x == 100, 'Expected 100')
