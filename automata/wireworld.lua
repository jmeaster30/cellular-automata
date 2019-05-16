win_title = "Wire World Automata"

win_width = 800
win_height = 800

cell_size = 20
num_of_states = 4

kernal_width = 3
kernal_height = 3

function process(curr_state, neighbor_counts)
  local new_state = 0

  if curr_state == 1 then
    new_state = 2
  elseif curr_state == 2 then
    new_state = 3
  elseif curr_state == 3 then
    if neighbor_counts[1] == 1 or neighbor_counts[1] == 2 then
      new_state = 1
    else
      new_state = 3
    end
  end

  return new_state
end
