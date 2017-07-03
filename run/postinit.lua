-------------------------------------------------------------------------------

print('post init')

-------------------------------------------------------------------------------

arche = pegr.find_archetype('cookie.at')
print(arche)

entity = pegr.new_entity(arche)
entity.position.x = 100
entity.position.y = 100
entity.velocity.x = 2
entity.velocity.y = 2

pegr.spawn(entity)

-------------------------------------------------------------------------------
