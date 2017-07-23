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

assert(ent.position.x == 315, 'Expected 315')

ent.position.x = 500

assert(ent.position.x == 500, 'Expected 500')

ent.position.x = 0
for i = 1,100 do
  ent.position.x = ent.position.x + 1
end

assert(ent.position.x == 100, 'Expected 100')
