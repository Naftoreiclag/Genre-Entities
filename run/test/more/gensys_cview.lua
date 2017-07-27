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
    local x = ent.position
  end
  pegr.debug_timer_end('Accessing cview', sample_size, 'ns')
end

-------------------------------------------------------------------------------

end
