-- theme files:
--
-- theme files need to define at least 2 variables: foreground and background
-- those 2 variables must be arrays (tables with numerical indices) that contain
-- the block positions within the png file of the theme. The first 2 values
-- of the array conain the coordinate of the first block, the next 2 the 2nd block
-- and so on. The first of the 2 variables is the block 2 position, the 2nd the
-- block y position. Blocks need to be at block positions (normally a block
-- is 40x48 pixels in size)
--
-- The order of the foreground table is fixed. That means the platform start always needs
-- to be the first image and the 4 image door animation the last 4 of the 23 images. To find
-- out what is required please look at existing theme.lua files
--
-- The background is just a list of tiles. Those tiles are then used by their number within
-- the level files. So once a tile number is given it must not be changed or given
-- levels might change appearance. But it is always possible to add more tiles to the end
-- of the list
--
-- A 3rd variable may be defined containing a table with a function for active tiles: activeTiles.
-- To explain this let's start from the other end. For each active tile pushover will call
-- a function for each frame. This function may change the image for the active tile. All
-- state connected to this tile will need to be stored within the function. Pushover will 
-- _not_ give any parameters to this function when it is called.
--
-- This is achieved using the Lua closures (please read the lua anual or the lua book). So
-- what we put into the activeTiles table is a function that returns that function that is called
-- by pushover with each frame. The state associated with each instance of the active tile
-- is defined in the closure




-- generator function for the closure generator function for a simple animation
-- the tiles to play while animating is given as parameter (an array)
function simpleAnimation(animationTiles)

    return function(x, y)
        -- local variables for the closure
        local ticker = 0
        local state = 0
        local animation = 1

        return function(action)
            if (state == 0) then          -- the inactive state, increment counter to increase
                ticker = ticker + 1       -- probability for activity
                if (rand() < ticker) then
                    state = 1
                    animation = 1
                end
            else if (state == 1) then
                animation = animation + 1
                if animation > #animationTiles then
                    animation = 1
                    state = 0
                    ticker = 0
                end
            end

            return animationTiles[animation]
        end
    end
end

-- for tiles that need to animate in concert, the leader needs to
-- be zero for the leading tile, for the others give the leader
function simpleAnimationInConcert(animationTiles, leader)

    return function(x, y)
        -- local variables for the closure
        local ticker = 0
        local state = 0
        local animation = 1

        return function(action)
            if (state == 0) then          -- the inactive state, increment counter to increase
                ticker = ticker + 1       -- probability for activity
                if (rand() < ticker) then
                    state = 1
                    animation = 1
                end
            else if (state == 1) then
                animation = animation + 1
                if animation > #animationTiles then
                    animation = 1
                    state = 0
                    ticker = 0
                end
            end

            return animationTiles[animation]
        end
    end
end

