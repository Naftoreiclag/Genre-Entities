return function(sample_size)

sample_size = sample_size or 100000

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

-------------------------------------------------------------------------------

local arche = pegr.find_archetype('simplex.at')

local ent = pegr.new_entity(arche)

-------------------------------------------------------------------------------

print('sample size: ', sample_size)

do
  ent.position.x = 0
  pegr.debug_collect_garbage()
  pegr.debug_timer_start()
  for i=1,sample_size,1 do
    ent.position.x = ent.position.x + 1
  end
  pegr.debug_timer_end('Simple member increment', sample_size, 'ns')
end

-------------------------------------------------------------------------------

do
  local cached = ent.position
  cached.x = 0
  pegr.debug_collect_garbage()
  pegr.debug_timer_start()
  for i=1,sample_size,1 do
    cached.x = cached.x + 1
  end
  pegr.debug_timer_end('Cached member increment', sample_size, 'ns')
end

-------------------------------------------------------------------------------

do
  local fake_ent = {}
  local x = 0
  setmetatable(fake_ent, {
    __index = function(t, k) return x end,
    __newindex = function(t, k, v) x = v end
  })
  fake_ent.x = 0
  pegr.debug_collect_garbage()
  pegr.debug_timer_start()
  for i=1,sample_size,1 do
    fake_ent.x = fake_ent.x + 1
  end
  pegr.debug_timer_end('Lua metatable-powered increment', sample_size, 'ns')
end

-------------------------------------------------------------------------------

do
  local fake_ent = {}
  fake_ent.x = 0
  pegr.debug_collect_garbage()
  pegr.debug_timer_start()
  for i=1,sample_size,1 do
    fake_ent.x = fake_ent.x + 1
  end
  pegr.debug_timer_end('Lua table member increment', sample_size, 'ns')
end

-------------------------------------------------------------------------------

do
  local x = 0
  pegr.debug_collect_garbage()
  pegr.debug_timer_start()
  for i=1,sample_size,1 do
    x = x + 1
  end
  pegr.debug_timer_end('Lua table member increment', sample_size, 'ns')
end

-------------------------------------------------------------------------------

end