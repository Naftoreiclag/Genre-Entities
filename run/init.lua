print('init')

pegr.add_component('position.c', {
  x = {'f64', 0},
  y = {'f64', 0},
  is_at_origin = {'func', function(self)
    return self.x == 0 and self.y == 0
  end},
})

pegr.add_component('velocity.c', {
  x = {'f64', 0},
  y = {'f64', 0},
  is_stationary = {'func', function(self)
    return self.x == 0 and self.y == 0
  end},
})

pegr.add_component('circle.c', {
  radius = {'f32', 1},
})

pegr.add_component('edible.c', {
  food_value = {'f32', 0},
})

pegr.add_archetype('cookie.at', {
  position = {
    -- (Resource ids should never be keys)
    __is = 'position.c',
    
    -- Start at (10, 10) for no particular reason
    x = {'f64', 10},
    y = {'f64', 10},
  },
  velocity = {
    __is = 'velocity.c',
    -- Keep all default default values
  },
  body = {
    __is = 'circle.c',
    
    -- Cookies are small
    radius = {'f32', 0.5},
  },
  edible = {
    __is = 'edible.c',
    
    -- 0.315 foodiness (this is a VERY hearty cookie)
    food_value = {'f32', 0.315},
  },
})


--[[
  Todo: allow for "super" types for primitives.
    For example, 'fany' can accept 'f32' or 'f64'
                 'iany' can accept 'i32' or 'i64'
]]
pegr.add_genre('physical.g', {
  interface = {
    pos_x = 'f64',
    pos_y = 'f64',
    is_at_origin = 'func',
    vel_x = 'f64',
    vel_y = 'f64',
    is_stationary = 'func',
    on_eaten = 'func',
  },
  
  implement = {
    position = {
      group = {'pos_x', 'pos_y', 'is_at_origin'},
      patterns = {
        --[[function(arche)
          pos_symb = arche:get_symbol('position.c')
          if pos_symb then
            return {
              pos_x = {'alias', pos_symb .. '.x'},
              pos_y = {'alias', pos_symb .. '.y'},
              is_at_origin = {'alias', pos_symb .. '.is_at_origin'},
            }
          end
        end,]]
        {
          __if = 'position.c',
          pos_x = {'alias', 'x'},
          pos_y = {'alias', 'y'},
          is_at_origin = {'alias', 'is_at_origin'},
        },
      },
    },
    velocity = {
      group = {'vel_x', 'vel_y', 'is_stationary'},
      patterns = {
        --[[function(arche)
          vel_symb = arche:get_symbol('velocity.c')
          if vel_symb then
            return {
              vel_x = {'alias', vel_symb .. '.x'},
              vel_y = {'alias', vel_symb .. '.y'},
              is_stationary = {'alias', vel_symb .. '.is_stationary'},
            }
          end
        end,]]
        {
          __if = 'velocity.c',
          vel_x = {'alias', 'x'},
          vel_y = {'alias', 'y'},
          is_stationary = {'alias', 'is_stationary'},
        },
      },
    },
    edible = {
      group = {'on_eaten'},
      patterns = {
        --[[function(arche)
          edible_symb = arche:get_symbol('edible.c')
          if edible_symb then
            return {
              on_eaten = {'alias', edible_symb .. '.on_eaten'},
            }
          end
        end,]]
        {
          __if = {'alias', 'edible.c'},
          on_eaten = {'alias', 'food_value'},
        }
      },
      -- If none of the patterns match, use this instead.
      -- Careful, default values are read-only.
      default = {
        on_eaten = {'func', function(self)
          print(string.format(
              'I was eaten at x: %f y:%f', self.pos_x, self.pos_y))
        end},
      },
    },
  },
})
