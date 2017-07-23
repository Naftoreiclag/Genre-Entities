--@Name Gensys test Lua garbage collection

-- This test also makes sure that the Gensys
-- ecosystem can be restarted on demand.

pegr.add_component('position.c', {
  x = {'f64', 315},
  y = {'f64', 630},
})

pegr.add_archetype('simplex.at', {
  position = {
    __is = 'position.c',
  }
})

pegr.debug_stage_compile()

local arche = pegr.find_archetype('simplex.at')

assert(arche, 'Arche not found!')

local ent = pegr.new_entity(arche)

assert(ent.__exists, 'Entity does not exist!')

pegr.delete_entity(ent)

assert(not ent.__exists, 'Entity still exists!')

