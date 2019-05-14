# cellular-automata

This is a cellular automata simulator that is run from the command line but uses SDL2 for drawing to the screen.

You need to pass in a lua script that has the parameters for the cellular automata. The file conwaysgol.lua is a lua script that implements the rules for Conway's Game of Life.

Your lua script should contain these variables:
```lua
win_title = "Title of the window"

win_width = 800 -- the width in pixels of the window
win_height = 800 -- the height in pixels of the window

cell_size = 40 -- the width and height of one cell (you should make this a divisor of both win_width and win_height otherwise you will get a funky looking grid)

kernal_width = 3 -- the width of the kernal or neighborhood centered on the current cell
kernal_height = 3 -- the height of the kernal or neighborhood centered on the current cell

updates_per_second = 60 -- this controls how frequently the grid updates
```
Your lua script should also have a function called **process** that takes in two parameters (an integer and an array) and returns an integer.

Here is how it can look:
```lua
--curr_state will be an integer representing the state of the cell being processed
--neighbor_counts will be an array of length equal to the number of states and each index will contain the number of neighbors of the current cell that have that state

function process(curr_state, neighbor_counts)
  local new_state = 0
  
  --you can have anything you want in here
  --as long as you return a number that represents a certain state 
  
  return new_state
end
```

