-------------------------------------------------------------------------------

print('post init')

-------------------------------------------------------------------------------

cookie_at = pegr.find_archetype('cookie.at')
print('cookie_at ' .. tostring(cookie_at))
pizza_at = pegr.find_archetype('pizza.at')
print('pizza_at ' .. tostring(pizza_at))
ball_at = pegr.find_archetype('bowling_ball.at')
print('ball_at ' .. tostring(ball_at))

local cookie1 = pegr.new_entity(cookie_at)
print('cookie1 ' .. tostring(cookie1))

local cookie2 = pegr.new_entity(cookie_at)
print('cookie2 ' .. tostring(cookie2))

local pizza1 = pegr.new_entity(pizza_at)
print('pizza1 ' .. tostring(pizza1))

local bb1 = pegr.new_entity(ball_at)
print('bb1 ' .. tostring(bb1))

cookie1.position.x = 100
cookie1.position.y = 100
cookie1.velocity.x = 2
cookie1.velocity.y = 2

--[[
"entity" is not yet a "real" entity until it has been spawned into the
active entity ecosystem. Until it is spawned in, it is an ordinary archetypical
view on an entity that also frees the associated memory when gc'd
]]

pegr.spawn(cookie1)

-------------------------------------------------------------------------------
