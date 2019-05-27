# cellular-automata

This is a cellular automata simulator that is run from the command line but uses OpenGL for drawing to the screen.

There are controls to help simulate your cellular automata:
```
Arrow Keys - Changes the selected state
WASD - Moves camera up, left, down, and right
Mouse Scroll - Zoom the camera in and out
Left Click - Sets the cell that the mouse is over to the selected state
C - sets every cell to state zero
R - sets every cell to a random state
E - performs the automaton calculation once
Space - Starts and Stops the simulation
```

You need to pass in a lua script that has the parameters for the cellular automata. In the automata folder are premade cellular automaton rules.
```
conwaysgol.lua  -> implements the rules for Conway's Game of Life.
brainsbrain.lua -> implements the rules for Brian's Brain.
wireworld.lua   -> implements the rules for Wire World.
langtonsant.lua -> implements the rules for Langton's Ant.
3staterules.lua -> implements the rules for a cellular automaton that I used for testing.
```

Your lua script can contain these variables but they aren't required (the values given are the default values):
```lua
win_title = "Cellular Automata" -- the title of the window

win_width = 640 -- the width in pixels of the window
win_height = 480 -- the height in pixels of the window

move_speed = 2.0 -- the movement speed of the camera
zoom_speed = 5.0 -- the zoom speed of the camera

cell_size = 20 -- the width and height of one cell
cells_wide = win_width / cell_size --the width of the grid in cells
cells_high = win_height / cell_size --the height of the grid in cells

num_of_states = 2 -- the number of states each cell can be
state_colors = {  -- the colors (in hex) of the state (low-high)
  "FFFFFF",       -- this array must have at least num_of_states number of elements
  "000000"
}

kernel_width = 3 -- the width of the kernel_width or neighborhood centered on the current cell
kernel_height = 3 -- the height of the kernel or neighborhood centered on the current cell

get_neighbors = false -- when false you get a lua array of the number of neighbors of each state. when true you get a 1d lua array of the entire kernel

updates_per_second = 60 -- this controls how frequently the grid updates
```
Your lua script must contain a function called **process** that takes in two parameters, an integer and an array (in that order), and returns an integer.

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
