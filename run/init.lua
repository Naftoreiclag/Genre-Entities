-------------------------------------------------------------------------------

print('init')

-------------------------------------------------------------------------------

pegr.add_component('position.c', {
  x = {'f64', 0},
  y = {'f64', 0},
  
  --[[ Important: The function type is implicitly static. This means
    that there are no per-entity functions. This is due to many reasons:
    - Serialization: When storing an entity into "raw" data, such as when
      writing to a save file or transmitting over the network, there is
      no safe and efficient means of storing a function.
    - Genre-specified functions: Genres can specify default values for
      static members, and functions should naturally be included in
      Genres. Therefore, to ease inclusion of functions in Genres,
      functions are implicitly static.
  
    The "self" argument is a component-view on the entity.
  ]]
  is_at_origin = {'func', function(self)
    return self.x == 0 and self.y == 0
  end},
})
print('added position.c')

pegr.add_component('velocity.c', {
  x = {'f64', 0},
  y = {'f64', 0},
  is_stationary = {'func', function(self)
    return self.x == 0 and self.y == 0
  end},
})
print('added velocity.c')

pegr.add_component('circle.c', {
  radius = {'f32', 1},
})
print('added circle.c')

pegr.add_component('edible.c', {
  food_value = {'f32', 0},
  on_eaten = {'func', function(self) end},
})
print('added edible.c')

-------------------------------------------------------------------------------

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
    
    -- The "self" argument is an archetype-view on the entity
    on_eaten = {'func', function(self)
      if food_value > 0.2 then
        print('This cookie sure is chewy!')
      else
        print('This cookie sure is sweet!')
      end
    end},
  },
})
print('added cookie.at')

pegr.add_archetype('pizza.at', {
  position = {
    -- (Resource ids should never be keys)
    __is = 'position.c',
    
    -- Start at (30, 10) for no particular reason
    x = {'f64', 30},
    y = {'f64', 10},
  },
  velocity = {
    __is = 'velocity.c',
    -- Keep all default default values
  },
  body = {
    __is = 'circle.c',
    
    -- Pizzas are big
    radius = {'f32', 2.0},
  },
  edible = {
    __is = 'edible.c',
    
    -- 0.63 foodiness (this is a deep dish)
    food_value = {'f32', 0.63},
    
    -- The "self" argument is an archetype-view on the entity
    on_eaten = {'func', function(self)
      if food_value > 0.2 then
        print('This pizza sure is juicy!')
      else
        print('This pizza sure is crispy!')
      end
    end},
  },
})
print('added pizza.at')

pegr.add_archetype('bowling_ball.at', {
  position = {
    __is = 'position.c',
  },
  velocity = {
    __is = 'velocity.c',
  },
  body = {
    __is = 'circle.c',
    radius = {'f32', 1.0},
  }
})
print('added bowling_ball.at')
    
-------------------------------------------------------------------------------

--[[
  Todo: allow for "super" types for primitives.
    For example, 'fany' can accept 'f32' or 'f64'
                 'iany' can accept 'i32' or 'i64'
  ... maybe?
  
  Alternatively, only allow floats and ints of predetermined size 
  (64 bit float and 32 bit int would work well)
]]
pegr.add_genre('food.g', {
  --[[ nil values indicate that those fields must be filled in by
    members of an archetype's components. Non-nils are optional
    and will remain that value if not replaced by another
    component. Since there cannot be any memory associated with
    non-nil default values, those default values must be static.
    Functions are always static, so specifying default
    functions does not break any read/write assumptions. Any
    primitive types that the client would ordinarily expect to
    write to, such as f64 or i32, CANNOT be specified within
    the genre, as write access is mandatory. Specifying anything
    other than nil as a value for such read/write types is an
    error.
  ]]
  interface = {
    pos_x = {'f64', nil},
    pos_y = {'f64', nil},
    is_at_origin = {'func', nil},
    vel_x = {'f64', nil},
    vel_y = {'f64', nil},
    is_stationary = {'func', nil},
    food_value = {'f32', nil},
    
    --[[ 
    -- Soon? static types
    essence = {'static i32', 100},
    ]]
    
    -- The "self" argument is a genre-view on the entity
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
    
    --[[ Functions can also be specified in this list, which accept
    Archetypes as arguments and output a table mapping aliases to
    Archetype members.
    
    maybe...?
    ]]
  },
})
print('added food.g')

-------------------------------------------------------------------------------

pegr.schedule_task('fizz.t', {
  
  -- Describes when this function should be called
  when = 'world tick',
  
  -- List of the function's arguments
  args = {
    entity = {'ent_view', 'food.g'}
  },
  
  -- The actual function to be called
  func = function(entity)
    
    -- Update position based on velocity
    entity.pos_x = entity.pos_x + entity.vel_x
    entity.pos_y = entity.pos_y + entity.vel_y
    
    
  end,
})
print('added fizz.t')

-------------------------------------------------------------------------------
