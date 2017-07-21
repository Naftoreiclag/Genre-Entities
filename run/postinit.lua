-------------------------------------------------------------------------------

print('post init')

-------------------------------------------------------------------------------

cookie_at = pegr.find_archetype('cookie.at')
print('cookie_at ' .. tostring(cookie_at))
pizza_at = pegr.find_archetype('pizza.at')
print('pizza_at ' .. tostring(pizza_at))
ball_at = pegr.find_archetype('bowling_ball.at')
print('ball_at ' .. tostring(ball_at))

for i=1,100,1 do
  local cookien = pegr.new_entity(cookie_at)
end

local cookie1 = pegr.new_entity(cookie_at)
print('cookie1 ' .. tostring(cookie1))

local cookie2 = pegr.new_entity(cookie_at)
print('cookie2 ' .. tostring(cookie2))

local pizza1 = pegr.new_entity(pizza_at)
print('pizza1 ' .. tostring(pizza1))

local bb1 = pegr.new_entity(ball_at)
print('bb1 ' .. tostring(bb1))

print('cookie1 id ' .. cookie1.__id)
print('cookie1 arche ' .. tostring(cookie1.__arche))
print('cookie1 killed ' .. tostring(cookie1.__killed))
print('cookie1 alive ' .. tostring(cookie1.__alive))
print('cookie1 spawned ' .. tostring(cookie1.__spawned))

print(cookie1.position)

print(cookie1.position.x)
print(cookie1.position.y)
print(cookie1.velocity.x)
print(cookie1.velocity.y)

cookie1.position.x = 100
cookie1.position.y = 100
cookie1.velocity.x = 2
cookie1.velocity.y = 2

--[[
"entity" is not yet a "real" entity until it has been spawned into the
active entity ecosystem. Until it is spawned in, it is an ordinary archetypical
view on an entity that also frees the associated memory when gc'd
]]

--pegr.spawn(cookie1)

-------------------------------------------------------------------------------
