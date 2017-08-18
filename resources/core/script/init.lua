-------------------------------------------------------------------------------

print('init')

-------------------------------------------------------------------------------

pegr.add_component('position.c', {
  --[[ Type must be specified explicitly. Default values are optional. Omitting
    a default value (second element is nil or only giving a raw type string
    instead of a table) requires all archetypes using this component to specify 
    their own default value for entity instantiation.
    
    As such, the parser will attempt to parse it as a type string before
    parsing it as a {type, value} pair
  ]] 
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
    Of course, the name of the argument doesn't matter, only its position.
  ]]
  is_at_origin = {'func', function(self)
    --print("Hello from is_at_origin!")
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
    
    --[[ For archetype primitives, only the value must be specified explicitly,
      as the type is already specified in the component. Any component member
      did not specify a default value must be given a value by every archetype
      which uses it.
    
      As such, the parser will attempt to parse it as a value literal before
      parsing it as a {type, value} pair
    ]]
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
    
    -- The "self" argument is still a component view. 
    -- Use __arche to retrieve the underlying archetype
    on_eaten = {'func', function(self)
      if self.food_value > 0.2 then
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
    
    -- Start at 30, 10 for no particular reason
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
    
    -- The "self" argument is still a component view. 
    -- Use __arche to retrieve the underlying archetype
    on_eaten = {'func', function(self)
      if self.food_value > 0.2 then
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
-- Soon? static types
essence = 'static i32',
]]
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
  
    -- Specifying a value for static members allows for optional members
    on_eaten = {'func', function(self)
      print(string.format(
          'I was eaten at x: %f y:%f', self.pos_x, self.pos_y))
    end},
  },
  
  -- Patterns are matched from the least index to the greatest.
  -- Indices can be negative, zero, or decimal. The first pattern
  -- to match is the one applied to the entity.
  patterns = {
    {
      matching = {
        position = 'position.c',
        velocity = 'velocity.c',
        edible = 'edible.c',
      },
      aliases = {
        pos_x = 'position.x',
        pos_y = 'position.y',
        is_at_origin = 'position.is_at_origin',
        vel_x = 'velocity.x',
        vel_y = 'velocity.y',
        -- Note that aliasing functions requires wrapping it in a
        -- c closure to replace the genre view with a cview
        is_stationary = 'velocity.is_stationary',
        on_eaten = 'edible.on_eaten',
        food_value = 'edible.food_value',
      },
      
      -- Static members can also be redefined. Functions take a
      -- genre view of the entity.
      --[[
      static = {
        on_eaten = {'func', function(self)
          print('on_eaten called through a genre!')
        end},
      },
      ]]
    },
  },
})
print('added food.g')

-------------------------------------------------------------------------------

pegr.add_event('custom_event.ev', {
  action = function(listeners)
    print('Custom event triggered!')
  end
})

local wi_listener = pegr.hook_listener({
  on = 'world_init.ev',
  func = function(world) 
    print('world_init')
    pegr.call_event('custom_event.ev')
  end,
})

local et_listener = pegr.hook_listener({
  
  -- On entity ticks
  on = 'entity_tick.ev',
  
  -- Start immediately
  delay = 0,
  
  -- Every time
  every = 1,
  
  --[[ (Everything other than 'on', 'func', and 'every' are extra 
  arguments, the meaning of which is determined by the choice of 'on')
  ]]
  
  -- Such as this one, which modifies which entities are selected:
  selector = 'food.g',
  
  -- The actual function to be called
  func = function(entity)
    
    -- Update position based on velocity
    entity.pos_x = entity.pos_x + entity.vel_x
    entity.pos_y = entity.pos_y + entity.vel_y
    
  end,
})
print('added fizz.t')

-------------------------------------------------------------------------------
