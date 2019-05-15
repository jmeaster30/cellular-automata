win_title = "3 State Cellular Automaton"
win_width = 800
win_height = 800

cell_size = 20
num_of_states = 3

kernal_width = 3
kernal_height = 3

function process(curr_state, neighbor_counts)
  local new_state = 0

  if neighbor_counts[1] < neighbor_counts[2] then
    new_state = 2
  elseif neighbor_counts[1] > neighbor_counts[2] then
    new_state = 1
  else
    new_state = 0
  end

  return new_state
end
