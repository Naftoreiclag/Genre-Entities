print('post init')

entity = pegr.Entity:new(pegr.find_archetype('cookie.at'))
entity.position.x = 100
entity.position.y = 100
entity.velocity.x = 2
entity.velocity.y = 2

pegr.spawn(entity)