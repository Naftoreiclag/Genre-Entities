-------------------------------------------------------------------------------

print('post init')

-------------------------------------------------------------------------------

cookie_at = pegr.find_archetype('cookie.at')
print('cookie_at ', cookie_at)
pizza_at = pegr.find_archetype('pizza.at')
print('pizza_at ', pizza_at)
ball_at = pegr.find_archetype('bowling_ball.at')
print('ball_at ', ball_at)

for i=1,100,1 do
  local cookien = pegr.new_entity(cookie_at)
end

local cookie1 = pegr.new_entity(cookie_at)
print('cookie1 ', cookie1)

local cookie2 = pegr.new_entity(cookie_at)
print('cookie2 ', cookie2)

local pizza1 = pegr.new_entity(pizza_at)
print('pizza1 ', pizza1)

local bb1 = pegr.new_entity(ball_at)
print('bb1 ', bb1)

print('cookie1 id ', cookie1.__id)
print('cookie1 arche ', cookie1.__arche)
print('cookie1 killed ', cookie1.__killed)
print('cookie1 alive ', cookie1.__alive)
print('cookie1 spawned ', cookie1.__spawned)

print('cookie1 position component ', cookie1.position)

print('cookie1 pos x ', cookie1.position.x)
print('cookie1 pos y ', cookie1.position.y)
print('cookie1 vel x ', cookie1.velocity.x)
print('cookie1 vel y ', cookie1.velocity.y)

cookie1.position.x = 999
cookie1.position.y = 100
cookie1.velocity.x = 2
cookie1.velocity.y = 2

local test = cookie1.position.x
print('original value: ', cookie1.position.x)
test = test + 1
print('value should be unchanged: ', cookie1.position.x)
cookie1.position.x = test
print('value should be different: ', cookie1.position.x)

print('cookie1 pos x ', cookie1.position.x)
print('cookie1 pos y ', cookie1.position.y)
print('cookie1 vel x ', cookie1.velocity.x)
print('cookie1 vel y ', cookie1.velocity.y)

print(cookie1.position.is_at_origin)

cookie1.position.x = 5
cookie1.position.y = 4
print(cookie1.position:is_at_origin())
assert(not cookie1.position:is_at_origin())

cookie1.position.x = 0
cookie1.position.y = 0
print(cookie1.position:is_at_origin())
assert(cookie1.position:is_at_origin())

-- Can't do this
--cookie1.position.is_at_origin = function() end

--[[
"entity" is not yet a "real" entity until it has been spawned into the
active entity ecosystem. Until it is spawned in, it is an ordinary archetypical
view on an entity that also frees the associated memory when gc'd
]]

pegr.spawn(cookie1)

-------------------------------------------------------------------------------
