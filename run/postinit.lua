-------------------------------------------------------------------------------

print('post init')

-------------------------------------------------------------------------------

arche = pegr.find_archetype('cookie.at')
print(arche)

local entity = pegr.new_entity(arche)
entity.position.x = 100
entity.position.y = 100
entity.velocity.x = 2
entity.velocity.y = 2

--[[
"entity" is not yet a "real" entity until it has been spawned into the
active entity ecosystem. Until it is spawned in, it is an ordinary archetypical
view on an entity that also frees the associated memory when gc'd
]]

pegr.spawn(entity)

-------------------------------------------------------------------------------
