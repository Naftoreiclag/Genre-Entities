-------------------------------------------------------------------------------

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
  on_eaten = {'func', function(self) end}
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
pegr.add_genre('food.g', {
  -- nil values indicate that those fields must be filled in by
  -- members of an archetype's components. Non-nils are optional
  -- and will remain that value if not replaced by another
  -- component
  interface = {
    pos_x = {'f64', nil},
    pos_y = {'f64', nil},
    is_at_origin = {'func', nil},
    vel_x = {'f64', nil},
    vel_y = {'f64', nil},
    is_stationary = {'func', nil},
    food_value = {'f32', nil},
    on_eaten = {'func', function(self)
      print(string.format(
          'I was eaten at x: %f y:%f', self.pos_x, self.pos_y))
    end}
  },
  patterns = {
    {
      __from = 'position.c',
      pos_x = 'x',
      pos_y = 'y',
      is_at_origin = 'is_at_origin',
    },
    {
      __from = 'velocity.c',
      vel_x = 'x',
      vel_y = 'y',
      is_stationary = 'is_stationary',
    },
    {
      __from = 'edible.c',
      on_eaten = 'on_eaten',
      food_value = 'food_value',
    },
  },
})
