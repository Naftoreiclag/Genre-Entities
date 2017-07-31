--@Name Gensys genre matching
pegr.add_component('position.cp', {
  x = {'f64', 17},
  y = {'f64', 19},
})

pegr.add_component('flavor.cp', {
  flavor = {'str', 'sweet'},
  strength = {'f64', 1.0},
})

pegr.add_archetype('cube.at', {
  location = {
    __is = 'position.cp',
  },
  taste = {
    __is = 'flavor.cp',
    flavor = {'str', 'salty'},
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

local comp = pegr.find_component('flavor.cp')
local arche = pegr.find_archetype('cube.at')
local genre = pegr.find_genre('food.gn')
local ent = pegr.new_entity(arche)

print('objs ---')
print(comp)
print(arche)
print(genre)
print(ent)

local cview = comp(ent)
local aview = arche(ent)
local gview = genre(ent)

print('views ---')
print(cview)
print(aview)
print(gview)

cview.flavor = 'tangy'
cview.strength = 5.0

aview.location.x = 628
aview.location.y = 271

assert(gview.taste == 'tangy')
assert(gview.power == 5.0)
assert(gview.pos_x == 628)
assert(gview.pos_y == 271)
