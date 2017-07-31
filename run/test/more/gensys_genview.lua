return function(sample_size)

sample_size = sample_size or 100000
pegr.add_component('position.cp', {
  x = {'f64', 17},
  y = {'f64', 19},
})

pegr.add_component('flavor.cp', {
  flavor = {'str', 'sweet'},
  strength = {'f64', 1.0},
})

pegr.add_archetype('fruit.at', {
  location = {
    __is = 'position.cp',
  },
  taste = {
    __is = 'flavor.cp',
    flavor = {'str', 'fruity'},
  },
})

pegr.add_genre('food.gn', {
  interface = {
    pos_x = {'f64', nil},
    pos_y = {'f64', nil},
    taste = {'str', nil},
    power = {'f64', nil},
  },
  
  patterns = {
    {
      matching = {
        position = 'position.cp',
        flavor = 'flavor.cp',
      },
      
      aliases = {
        pos_x = 'position.x',
        pos_y = 'position.y',
        
        taste = 'flavor.flavor',
        power = 'flavor.strength',
      },
    },
  },
})

pegr.debug_stage_compile()

-------------------------------------------------------------------------------

local arche = pegr.find_archetype('fruit.at')
local comp = pegr.find_component('position.cp')
local genre = pegr.find_genre('food.gn')
local ent = pegr.new_entity(arche)

-------------------------------------------------------------------------------

print('sample size: ', sample_size)

do
  local gview = genre(ent)
  assert(gview)
  gview.pos_x = 0
  pegr.debug_collect_garbage()
  pegr.debug_timer_start()
  for i=1,sample_size,1 do
    gview.pos_x = gview.pos_x + 1
  end
  pegr.debug_timer_end('Increment member, gview', sample_size, 'ns')
end

do
  local cview = comp(ent)
  assert(cview)
  cview.x = 0
  pegr.debug_collect_garbage()
  pegr.debug_timer_start()
  for i=1,sample_size,1 do
    cview.x = cview.x + 1
  end
  pegr.debug_timer_end('Increment member, cview', sample_size, 'ns')
end

-------------------------------------------------------------------------------

end
