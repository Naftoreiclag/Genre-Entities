print('init')

pegr.add_component('position.c', {
    x = {'f64', 0},
    y = {'f64', 0},
    is_at_origin = {'func', function()
        return x == 0 and y == 0
    end}
})

pegr.add_component('velocity.c', {
    x = {'f64', 0},
    y = {'f64', 0},
    is_stationary = {'func', function()
        return x == 0 and y == 0
    end}
})

pegr.add_component('circle.c', {
    radius = {'f32', 1}
})

pegr.add_archetype('cookie.at', {
    position = {
        _is = 'position.c',
        
        -- Start at (10, 10) for no particular reason
        x = 10,
        y = 10
    },
    velocity = {
        _is = 'velocity.c'
        -- Keep all default default values
    },
    body = {
        _is = 'circle.c',
        
        -- Cookies are small
        radius = 0.5
    }
})
